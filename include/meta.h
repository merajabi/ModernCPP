template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

template < typename A, typename B >
struct is_same {
    static const bool value = false;
};

template < typename A >
struct is_same<A, A> {
    static const bool value = true;
};

/*
template < typename T, typename dummy = T >
struct remap {
    int foo(int x){
		return x;
	}
};

template < typename T >
struct remap < T, typename enable_if< is_same<T, unsigned char>::value
                        			|| is_same<T, signed char>::value, T >::type > {
    int foo(int x){
		return x*x;
	}
};
*/
