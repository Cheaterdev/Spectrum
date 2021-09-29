#pragma once

template<typename T> concept HasData =
requires (T t) {
	t.data;
};


template<typename T> concept HasBindless =
requires (T t) {
	t.bindless;
};


template<typename T> concept HasSRV =
requires (T t) {
	t.srv;
};

template<typename T> concept HasUAV =
requires (T t) {
	t.uav;
};

template<typename T> concept HasSMP =
requires (T t) {
	t.smp;
};

template<typename T> concept HasCB =
requires (T t) {
	t.cb;
};

template<typename T> concept HasRTV =
requires (T t) {
	t.rtv;
};

template<typename T> concept HasDSV =
requires (T t) {
	t.dsv;
};


template<typename T> concept TableHasSRV =
requires (T t) {
	t.SRV;
};

template<typename T> concept TableHasUAV =
requires (T t) {
	t.UAV;
};

template<typename T> concept TableHasSMP =
requires (T t) {
	t.SMP;
};

template<typename T> concept TableHasCB =
requires (T t) {
	t.CB;
};



template<typename T> concept HasCBType =
requires () {
	T::CB ;
};



template<class T>
struct _Underlying
{
	using TYPE = T;
};

template<HasCB T>
struct _Underlying<T>
{
	using TYPE = typename T::CB;
};

template<class T>
using Underlying = typename _Underlying<T>::TYPE;