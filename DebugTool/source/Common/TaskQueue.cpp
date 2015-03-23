#include <Windows.h>
#include "TaskQueue.h"

using namespace SoraDbgPlot::Task;

volatile unsigned long SoraDbgPlot::Task::TaskQueue::__countQueue = 0;
HANDLE SoraDbgPlot::Task::TaskQueue::__eventWait = ::CreateEvent(NULL, FALSE, TRUE, NULL);
volatile bool SoraDbgPlot::Task::TaskQueue::__clean = false;

#ifdef _DEBUG
volatile unsigned long SoraDbgPlot::Task::TaskQueue::__taskCntInQueue = 0;
volatile unsigned long SoraDbgPlot::Task::TaskQueue::__taskCntToRun = 0;
volatile unsigned long SoraDbgPlot::Task::TaskQueue::__taskCntRunning = 0;
volatile unsigned long SoraDbgPlot::Task::TaskQueue::__taskQueueOpCount = 0;
#endif

volatile unsigned long SoraDbgPlot::Task::TaskQueue::__sid = 0;
volatile unsigned long SoraDbgPlot::Task::TaskQueue::__taskNum = 0;
