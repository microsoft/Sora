#pragma once

#include <memory>

namespace SoraDbgPlot { namespace Task {

	class Runnable : public std::enable_shared_from_this<Runnable>
	{
	public:
		virtual void Run() = 0;
		virtual std::shared_ptr<Runnable> ContinueWith(std::shared_ptr<Runnable>) = 0;
		virtual ~Runnable() {}
	};

}}
