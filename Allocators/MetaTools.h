#pragma once

template<bool CONDITION, typename THEN, typename ELSE>
class IF__ {};
template<typename THEN, typename ELSE>
class IF__<true, THEN, ELSE> { public:typedef THEN reType; };
template<typename THEN, typename ELSE>
class IF__<false, THEN, ELSE> { public:typedef ELSE reType; };

template<template<typename STATEMENT>class CONDITION, typename STATEMENT>
class WHILE__ {
	template<typename STATEMENT> class STOP { public: typedef STATEMENT reType; };
public:
	typedef typename
		IF__<CONDITION<STATEMENT>::ret,
		WHILE__<CONDITION, typename STATEMENT::NEXT>,
		STOP<STATEMENT>>::reType::reType
		reType;
};
