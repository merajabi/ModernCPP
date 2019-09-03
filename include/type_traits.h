#ifndef _ModernCPP_meta_H_
#define _ModernCPP_meta_H_

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
		typedef typename ModernCPP::remove_volatile<typename ModernCPP::remove_const<T>::type>::type type;
	};

	template< class T >	struct add_cv { typedef const volatile T type; };
	template< class T > struct add_const { typedef const T type; };
	template< class T > struct add_volatile { typedef volatile T type; };

	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	//template< class T > struct remove_reference<T&&> {typedef T type;};

	template< class T >
	struct remove_cvref {
		typedef typename ModernCPP::remove_cv<typename ModernCPP::remove_reference<T>::type>::type type;
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
	};

	template< typename U >
	reference_wrapper<U> ref(reference_wrapper<U> x){
		return x;
	};

	template<typename T,typename U>
	class _Bind {
		T func;
		U param;
		public:
		_Bind(T t,U u):func(t),param(u){};
		void operator ()(){
			func(param);
		}
	};

	template<typename T,typename U>
	_Bind<T,U> bind(T t,U u){
		return _Bind<T,U>(t,u);
	}

	template<typename T> struct autotypeof { typedef T type; };

};
#endif //_ModernCPP_meta_H_

