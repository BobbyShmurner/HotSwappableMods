#pragma once

template<class T> struct PtrLessThan {
	bool operator()(T* lhs, T* rhs) const {
		return *lhs < *rhs; }
};