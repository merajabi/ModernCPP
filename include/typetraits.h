#ifndef _ModernCPP_meta_H_
#define _ModernCPP_meta_H_

#if (__cplusplus < 201103L)
	#ifndef nullptr
		#ifndef NULL
			#define NULL 0
		#endif
		#define nullptr NULL
	#endif
	#define AUTO(x,y) autotypeof<__typeof__(y)>::type x = y
#else
	#define AUTO(x,y) auto x = y
#endif 

namespace ModernCPP {

	template<bool B, class T = void>
	struct enable_if {};

	template<class T>
	struct enable_if<true, T> { typedef T type; };

	template < typename A, typename B >
	struct is_same { static const bool value = false; };

	template < typename A >
	struct is_same<A, A> { static const bool value = true; };

	template< class T > 
	struct remove_const          { typedef T type; };

	template< class T > 
	struct remove_const<const T> { typedef T type; };
	 
	template< class T > 
	struct remove_volatile             { typedef T type; };

	template< class T > 
	struct remove_volatile<volatile T> { typedef T type; };

	template< class T >
	struct remove_cv { 
		typedef typename remove_volatile<typename remove_const<T>::type>::type type;
	};

	template< class T >	struct add_cv { typedef const volatile T type; };
	template< class T > struct add_const { typedef const T type; };
	template< class T > struct add_volatile { typedef volatile T type; };

	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	//template< class T > struct remove_reference<T&&> {typedef T type;};

	template< class T >
	struct remove_cvref {
		typedef typename remove_cv<typename remove_reference<T>::type>::type type;
	};

	template< class T >
	T* addressof(T& arg) 
	{
		return reinterpret_cast<T*>(
		           &const_cast<char&>(
		              reinterpret_cast<const volatile char&>(arg)));
	}

	template <typename T>
	class reference_wrapper {
		T* _ptr;
	public:
	  // types
	  //typedef T type;
	 
		reference_wrapper(T& u) : _ptr(ModernCPP::addressof(u)) {};


	  //reference_wrapper(const reference_wrapper&) noexcept = default;
	  // assignment
	  //reference_wrapper& operator=(const reference_wrapper& x) noexcept = default;
	 
	  // access
		operator T& () const { return *_ptr; }
		T& get() const { return *_ptr; }
	 /*
	  template< class... ArgTypes >
	  std::invoke_result_t<T&, ArgTypes...>
		operator() ( ArgTypes&&... args ) const {
		return std::invoke(get(), std::forward<ArgTypes>(args)...);
	  }
	 */

		T* operator ->() {
			return _ptr;
		}
		T* operator ->() const {
			return _ptr;
		}

	};
	// deduction guides
	//template<class T>
	//reference_wrapper(T&) -> reference_wrapper<T>;

	template< typename U >
	reference_wrapper<U> ref(U& x){
		return reference_wrapper<U>(x);
	}

	template< typename U >
	reference_wrapper<U> ref(reference_wrapper<U> x){
		return x;
	}

	template <typename T>
	class right_reference {
		T* _ptr;
	public:

		right_reference(T& u) : _ptr(addressof(u)) {};

		T* operator ->() const {
			return _ptr;
		}
	};

	template< typename U >
	right_reference<U> refmove(U& u){
		return right_reference<U>(u);
	}

	template< typename U >
	right_reference<U> refmove(right_reference<U> u){
		return u;
	}

	template <typename F,typename U> struct _Bind;

	template <typename R, typename P,typename U>
	class _Bind<R (&)(P),U> {
		R (&func)(P) ;
		U param;

		_Bind();
		public:
		_Bind(R (&f)(P),U u):func(f),param(u){};
		R operator ()(){
			return func(param);
		}
	};
	template <typename R>
	class _Bind<R (&)(void),void> {
		R (&func)(void) ;

		_Bind();
		public:
		_Bind(R (&f)(void)):func(f){};
		R operator ()(){
			return func();
		}
	};


	template <typename T, typename R, typename P>
	class _Bind<R (T::*)(P),void> {
		T* obj;
		R (T::*func)(P) ;

		_Bind();
		//Delegate( const Delegate& );
	public:
		_Bind(R (T::*mf)(P),T* obj) : obj(obj), func(mf){
		}

		R operator()(P p) {
		    return (obj->*func)(p);
		}
	};
	template <typename T, typename R>
	class _Bind<R (T::*)(void),void> {
		T* obj;
		R (T::*func)(void) ;

		_Bind();
		//Delegate( const Delegate& );
	public:
		_Bind(R (T::*mf)(void),T* obj) : obj(obj), func(mf){
		}

		R operator()(void) {
		    return (obj->*func)();
		}
	};

	template<typename R,typename P,typename U>
	_Bind<R (&)(P),U> bind(R (&f)(P),U u){
		return _Bind<R (&)(P),U>(f,u);
	}
	template<typename R>
	_Bind<R (&)(void),void> bind(R (&f)(void)){
		return _Bind<R (&)(void),void>(f);
	}

	template <typename T, typename R, typename P>
	_Bind<R (T::*)(P),void> bind(R (T::*mf)(P),T* obj) {
		//return (obj.*mf)(std::forward<Args>(args)...);
		return _Bind<R (T::*)(P),void>(mf, obj);
	}
	template <typename T, typename R>
	_Bind<R (T::*)(void),void> bind(R (T::*mf)(void),T* obj) {
		return _Bind<R (T::*)(void),void>(mf, obj);
	}

	template<typename T> struct autotypeof { typedef T type; };

}
#endif //_ModernCPP_meta_H_

