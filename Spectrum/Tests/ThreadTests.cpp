#include "pch.h"

// Tasks
#include <functional>
#include <utility>

int CalculateParam1()
{
	// Simulate soft task
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return 4;
}

int CalculateParam2()
{
	// Simulate hard task
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 6;
}

void ProcessParams(int p1, int p2)
{
	int v = p1 + p2;

	v += 5;
}

void ProcessParamsEx(int p1, int b, int p2)
{
	int v = p1 + p2 + b;

	v += 5;
}

int aa(int b)
{
	return 1;
}

// Testing
template<typename _Ret = void>
class gtask : public base_task
{
	typedef std::packaged_task<_Ret()> _TaskType;
public:
	template<class _Fn, class... _Args>
	explicit gtask(_Fn&& _Fx, _Args&&... _Ax):
		execFunction(std::bind(_Decay_copy(std::forward<_Fn>(_Fx)),
		_Decay_copy(std::forward<_Args>(_Ax))...))
	{}

	void ProcessTask()
	{
		execFunction();
	}


	//std::future<_Ret> get_future() const { return g.get_future(); }
protected:
	_TaskType execFunction;
};

namespace bind_helpers
{
	template<typename G>
	G get(std::future<G>& member)
	{
		return member.get();
	}
	template<typename G>
	G get(G member)
	{
		return member;
	}
}

template<typename... _Types>
class deferred_bind;

template<typename T, typename... _Types>
class deferred_bind<T, _Types...> : public deferred_bind<_Types...>
{
public:
	deferred_bind(T mem, _Types... args) : deferred_bind<_Types...>(std::move(args)...), member(std::move(mem)) {}

	template<typename U, typename... _Args>
	void call(U& func, _Args... args)
	{
		auto arg = bind_helpers::get(member);
		deferred_bind<_Types...>::call(func, args..., arg);
	}
protected:
	T member;
};

template<typename T>
class deferred_bind<T>
{
public:
	deferred_bind(T mem) : member(std::move(mem)) {}

	template<typename U, typename... _Args>
	void call(U& func, _Args... args)
	{
		auto arg = bind_helpers::get(member);
		func(args..., arg);
	}
protected:
	T member;
};

// for function without parameters
template<>
class deferred_bind<>
{
public:
	deferred_bind() {}

	template<typename U>
	void call(U& func)
	{
		func();
	}
};

void DefBindTest1(int)
{}

void DefBindTest22(int a, int b, int c)
{
	a, b;
}

template<typename... _Args>
deferred_bind<_Args&...> make_deferred_bind(_Args... args)
{
	return std::move(deferred_bind<_Args&...>(args...));
}

template<class _Fn, class... _Args>
class gentask : public base_task
{
	typedef deferred_bind<_Args...> _TaskParameters;
	typedef std::packaged_task<_Fn> _TaskFunction;
public:
	explicit gentask(_Fn&& _Fx, _Args&&... args) :
		function(_Fx),
		params(std::move(args)...)
	{}
	explicit gentask(std::function<_Fn> _Fx, _Args&&... args) :
		function(_Fx),
		params(std::move(args)...)
	{}

	void ProcessTask()
	{
		params.call(function);//execFunction();
	}

	auto get_future()->std::future<int>
	{
		return function.get_future();
	}
	//template<typename U>
	//typename function::std::future<U> get_future() const { return function.get_future(); }
protected:
	_TaskParameters params;
	_TaskFunction function;
};

template<typename _Fn, typename... _Args>
std::shared_ptr<base_task> make_gentask(_Fn fn, _Args... args)
{
	return nullptr;
}

void TestTaskSequences()
{
	//gtask<void> t1(std::bind(ProcessParamsEx, 4));

	std::packaged_task<int(int)> b(std::bind(aa, std::placeholders::_1));
	b(4);
	std::future<int> f = b.get_future();
	
	//gtask<int> t2(func, 4);
	//deferred_bind<std::future<int>&> g(f);
	deferred_bind<std::future<int>&, int, int> g1(f, 6, 7);
	//auto g31 = make_deferred_bind(std::move(f), 6, 7);
	g1.call(DefBindTest22);
	//	g.build();
	//	g.build();

	{
		gentask<int()> task1(CalculateParam1);
		gentask<int()> task2([](){return 9; });
		//gentask<void(int, int), std::future<int>&, std::future<int>&> task3(ProcessParams, task1.get_future(), task2.get_future());

		//auto a = std::bind(ProcessParamsEx, std::placeholders::_2, 5, std::placeholders::_1);
		std::function<void(int,int)> a = std::bind(ProcessParams, std::placeholders::_1, std::placeholders::_2);
		gentask<void(int, int), int, int> task4(a, 4, 5);

		std::function<void(int, int)> aq = std::bind(ProcessParamsEx, std::placeholders::_2, -1, std::placeholders::_1);
		gentask<void(int, int), std::future<int>&, std::future<int>&> task5(aq, task1.get_future(), task2.get_future());

		make_gentask(a, 4, 6);

		//gentask<void(int, int), std::future<int>&, std::future<int>&> task5(aq, task1.get_future(), task2.get_future());
		


 		task1.ProcessTask();
 		task2.ProcessTask();
 		//task3.ProcessTask();
		task4.ProcessTask();
		task5.ProcessTask();
	}
}