#ifndef VEC_CUH
#define VEC_CUH

#include <cuda.h>

#include <iostream>

#include "macros.cuh"

//TODO:
//1. add some enable_if checks
//2. can we add "templated enums" for indexing data[...]: {x,y}, {r,g,b,a}...

template<int n, typename T>
class Vec{
public:
	__DEVICE__ __HOST__ Vec(){
		for(auto i=0; i<n; ++i)
			data[i] = 0.0;
	}

	template <typename ...Args>
	__DEVICE__ __HOST__ Vec(Args... args){
		auto sz = sizeof...(args);
		if(sz <= n){
			auto dp = data;
			for(auto val: {args...})
				*(dp++) = val;
			for(auto i=sz; i<n; ++i)
				*(dp++) = 0.0;
		}
		else{
			auto dp = data;
			for(auto val: {args...})
				if(dp-data < n)
					*(dp++) = val;
		}
	}

	__DEVICE__ __HOST__ Vec(T* const t){
		for(auto i=0; i<n; ++i)
			data[i] = t[i];
	}

	__DEVICE__ __HOST__ T& operator[](int const &index){
		return data[index];
	}

	__DEVICE__ __HOST__ T operator[](int const &index) const{
		return data[index];
	}

	__DEVICE__ __HOST__ friend std::ostream& operator<<(std::ostream &os, Vec<n, T> const &vec){
		for(auto i=0; i<n; ++i)
			os << vec[i] << "\t";
		return os;
	}

	//operator overrides
	__DEVICE__ __HOST__ Vec<n, T> operator*(T const &d) const{
		T tmp[n];
		for(auto i=0; i<n; ++i)
			tmp[i] = data[i] *d;
		return Vec<n, T>{tmp};
	}

	__DEVICE__ __HOST__ Vec<n, T> operator/(T const &d) const{
		T tmp[n];
		for(auto i=0; i<n; ++i)
			tmp[i] = data[i] /d;
		return Vec<n, T>{tmp};
	}

	__DEVICE__ __HOST__ Vec<n, T> operator+=(Vec<n, T> const &a){
		return *this = *this + a;
	}

	__DEVICE__ __HOST__ Vec<n, T> operator-=(Vec<n, T> const &a){
		return *this = *this - a;
	}

	__DEVICE__ __HOST__ Vec<n, T> operator*=(Vec<n, T> const &a){
		return *this = *this * a;
	}

	__DEVICE__ __HOST__ Vec<n, T> operator*=(T const &d){
		return *this = *this * d;
	}

	__DEVICE__ __HOST__ Vec<n, T> operator/=(T const &d){
		return *this = *this / d;
	}

	T data[n];
};

//unsymmetric operators
template<int n, typename T>
__DEVICE__ __HOST__ Vec<n, T> operator+(Vec<n, T> const &a, Vec<n, T> const &b){
	T tmp[n];
	for(auto i=0; i<n; ++i)
		tmp[i] = a[i] + b[i];
	return Vec<n, T>{tmp};
}

template<int n, typename T>
__DEVICE__ __HOST__ Vec<n, T> operator-(Vec<n, T> const &a, Vec<n, T> const &b){
	T tmp[n];
	for(auto i=0; i<n; ++i)
		tmp[i] = a[i] - b[i];
	return Vec<n, T>{tmp};
}

template<int n, typename T>
__DEVICE__ __HOST__ T operator*(Vec<n, T> const &a, Vec<n, T> const &b){
	T ret;
	for(auto i=0; i<n; ++i)
		ret += a[i] * b[i];
	return ret;
}

template<int n, typename T>
__DEVICE__ __HOST__ Vec<n, T> operator*(T const &d, Vec<n, T> const &b){
	return b * d;
}

//math transform functions
template<int n, typename T>
__DEVICE__ __HOST__ T length(Vec<n, T> const &a){
	return sqrt(a*a);
}

template<int n, typename T>
__DEVICE__ __HOST__ Vec<n, T> norm(Vec<n, T> const &a){
	auto len = length(a);
	if(len != 0.0)
		return a/len;
	else{
		printf("Error when norm vec: divided by zero.\n");
		return {0.0, 0.0};
	}
}


using vec2 = Vec<2, float>;
using vec3 = Vec<3, float>;
using vec4 = Vec<4, float>;

#endif