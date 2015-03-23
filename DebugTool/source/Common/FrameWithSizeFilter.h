#pragma once

#include <Windows.h>
#include "Writable.h"
#include "DynamicArray.h"
#include "RingBufferWithTimeStamp.h"
#include "Event.h"
#include "ErrMsg.h"

namespace SoraDbgPlot
{
	template <typename T>
	class FrameWithSizeInfoWriter
	{
	public:
		struct FlushDataEvent { char * ptr; size_t length; unsigned long long timestamp; };
		SoraDbgPlot::Event::Event<FlushDataEvent> EventFlushData;

	public:
		FrameWithSizeInfoWriter(RingBufferWithTimeStamp<T> * writable)
		{
			_writable = writable;
			_dynamicArray = new SoraDbgPlot::DynamicArray();
			Init();
		}
		
		FrameWithSizeInfoWriter()
		{
			_writable = 0;
			_dynamicArray = new SoraDbgPlot::DynamicArray();
			Init();
		}

		~FrameWithSizeInfoWriter()
		{
			delete _dynamicArray;
		}

		void Init()
		{
			_idExpected = 0;
			_sizeTarget = 0;
			_timestamp = (unsigned long long)-1;
			_bDiscardFlag = false;
			_bFirstPackage = true;
			_state = &_getSizeState;
		}

		void Write(const void * ptr, size_t size, size_t & sizeWritten)
		{
#ifdef DEBUG_INTERNAL_DATAFILTER
			_msg.Reset();
#endif
			assert(_state == &_getSizeState);

			char * ptrC = (char *)ptr;
			do {
				size_t sizeProcessed;
				_state->Handle(ptrC, size, sizeProcessed, this);
				ptrC += sizeProcessed;
				size -= sizeProcessed;
			} while(size > 0);
		}

	private:
		size_t _sizeTarget;
		RingBufferWithTimeStamp<T> * _writable;
		DynamicArray * _dynamicArray;
		unsigned long long _timestamp;

	private:

		void FlushBufferIfTestPassed()
		{
			if (_bDiscardFlag)
			{
#ifdef DEBUG_INTERNAL_DATAFILTER
				_msg.Append(L"Discard data: %d bytes\n", _dynamicArray->Size());
				ReportErrorState();
				_msg.Reset();
#endif	
				return;
			}

			if (_writable)
				_writable->Write((const T *)_dynamicArray->Ptr(), _dynamicArray->Size() / sizeof(T), _timestamp);
			FlushDataEvent e;
			e.ptr = (char *)_dynamicArray->Ptr();
			e.length = _dynamicArray->Size();
			e.timestamp = _timestamp;

			this->EventFlushData.Raise(this, e);

			_dynamicArray->Reset();
		}

		template <typename T>
		class State
		{
		public:
			virtual void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context) = 0;
		};

		template <typename T>
		class GetSizeState : public State<T>
		{
		public:
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
				const size_t sizeExpected = sizeof(__int16);

#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Reset();
				context->_msg.Append(L"h(g, %d)", size);
#endif

				if (size < sizeExpected)
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"e(1, %d)", size);
#endif
					context->_state = &context->_errorState;
					sizeProcessed = 0;
					return;
				}

				__int16 sizeTarget = *(__int16 *)ptr;
				
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"(%d, %d)", sizeTarget, sizeTarget % (64*sizeof(int)));
#endif	
				
				if (sizeTarget == 0)
				{
					sizeProcessed = sizeExpected;
					return;
				}
				else if ((int)sizeTarget == -1)
				{
					context->_dynamicArray->Reset();
					sizeProcessed = sizeExpected;
					return;
				}
				else
				{
					context->_sizeTarget = sizeTarget;
					context->_state = &context->_getIdState;
					sizeProcessed = sizeExpected;
					return;
				}
			}
		};

		template <typename T>
		class GetIdState : public State<T>
		{
		public:
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Reset();
				context->_msg.Append(L"h(id, %d)", size);
#endif

				size_t sizeExpected = sizeof(__int16);

				if (size < sizeExpected)
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"e(2, %d)", size);
#endif
					context->_state = &context->_errorState;
					sizeProcessed = 0;
					return;
				}

				__int16 id = *(__int16 *)ptr;
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"(%d)", id);
#endif	

				if (id == context->_idExpected)
					context->_idExpected++;
				else
				{
					context->_bDiscardFlag = true;
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"Id Error\n");
					context->_msg.Append(L"%d expected, %d got\n", context->_idExpected, id);
					context->ReportErrorState();
					context->_msg.Reset();
#endif
				}

				if (id == 0)	// first packet
				{
					context->_bDiscardFlag = false;
					context->_dynamicArray->Reset();
					context->_bFirstPackage = true;
				}
				else
					context->_bFirstPackage = false;

				context->_state = &context->_getTimeStampState;
				sizeProcessed = sizeExpected;
				return;
			}
		};

		template <typename T>
		class GetTimeStampState : public State<T>
		{
		public:
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
				size_t sizeExpected = sizeof(unsigned long long);

#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"h(t, %d)", size);
#endif
				if (size < sizeExpected)
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"e(1, %d)", size);
#endif
					context->_state = &context->_errorState;
					sizeProcessed = 0;
					return;
				}

				unsigned long long timestamp = *(unsigned long long *)ptr;

#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"(%I64d)", timestamp);
#endif
				if (context->_bFirstPackage)	// check timestamp increment
				{
					if ( (timestamp < context->_timestamp) && (context->_timestamp != (unsigned long long)(-1)) )
					{
#ifdef DEBUG_INTERNAL_DATAFILTER
						context->_msg.Append(L"timestamp Error\n");
						context->_msg.Append(L"last timestamp is %I64d, but current is %I64d\n", context->_timestamp, timestamp);
						context->ReportErrorState();
						context->_msg.Reset();
#endif
						context->_bDiscardFlag = true;
					}
				}
				else if ( (timestamp != context->_timestamp) ) // check if timestamp is the same
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"timestamp Error\n");
					context->_msg.Append(L"%I64d expected, %I64d got\n", context->_timestamp, timestamp);
					context->ReportErrorState();
					context->_msg.Reset();
#endif

					context->_bDiscardFlag = true;
				}

				context->_timestamp = timestamp;

				context->_state = &context->_saveDataState;
				sizeProcessed = sizeExpected;
				return;
			}
		};
		
		template <typename T>
		class SaveDataState : public State<T>
		{
		public:
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"h(s, %d)", size);
#endif

				size_t sizeToWrite = context->_sizeTarget 
					- sizeof(__int16) // id
					- sizeof(unsigned long long)	// timestamp
					- sizeof(char);	// controlFlag

				if (size < sizeToWrite)
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"e(1, %d)", sizeToWrite);
#endif
					context->_state = &context->_errorState;
					sizeProcessed = 0;
					return;
				}
				else
				{
					size_t discard;

					context->_dynamicArray->Write(ptr, sizeToWrite, discard);
					context->_state = &context->_getControlFlagState;
					sizeProcessed = sizeToWrite;
					return;
				}
			}
		};
		
		template <typename T>
		class GetControlFlagState : public State<T>
		{
		public:
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"h(c, %d)\n", size);
#endif


				if (size < sizeof(char))
				{
#ifdef DEBUG_INTERNAL_DATAFILTER
					context->_msg.Append(L"e(1, %d)", size);
#endif
					context->_state = &context->_errorState;
					sizeProcessed = 0;
					return;
				}

				char controlFlag = *(char *)ptr;
				if (controlFlag == 1)
				{
					context->FlushBufferIfTestPassed();
					context->_idExpected = 0;
				}

				context->_state = &context->_getSizeState;
				sizeProcessed = 1;
			}
		};

		template <typename T>
		class ErrorState : public State<T>
		{
			void Handle(
				const void * ptr, 
				size_t size, 
				size_t & sizeProcessed,
				FrameWithSizeInfoWriter<T> * context)
			{
#ifdef DEBUG_INTERNAL_DATAFILTER
				context->_msg.Append(L"Fatal Error!\n");
				context->_msg.Append(L"############\n");
				context->ReportErrorState();
#endif
				context->_dynamicArray->Reset();
				context->_state = &context->_getSizeState;
				sizeProcessed = size;
			}
		};


		State<T> * _state;
		GetSizeState<T> _getSizeState;
		GetIdState<T> _getIdState;
		GetTimeStampState<T> _getTimeStampState;
		SaveDataState<T> _saveDataState;
		GetControlFlagState<T> _getControlFlagState;
		ErrorState<T> _errorState;

		bool _bDiscardFlag;
		bool _bFirstPackage;
		__int16 _idExpected;

#ifdef DEBUG_INTERNAL_DATAFILTER
		void ReportErrorState()
		{
			::OutputDebugString(L"Data Filter error\n");
			::OutputDebugString((const wchar_t *)_msg);
		}
#endif
		
		SoraDbgPlot::Msg::Message _msg;
	};
}


