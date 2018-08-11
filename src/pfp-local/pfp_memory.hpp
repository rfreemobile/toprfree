#ifndef INCLUDE_pfp_memory
#define INCLUDE_pfp_memory

#include <exception>

namespace stdplus {


/// non-owning pointer (also: can be freely reseated to point at other address)
template <typename T>
class ptr_only {
	public:
		ptr_only(T * p=nullptr)
			: m_p(p) { }

		ptr_only<T> & operator=(const ptr_only<T> & other) noexcept {
			this->m_p = other.m_p;
			return *this;
		}

		typename std::add_lvalue_reference<T>::type operator*() { return *m_p; }
		typename std::add_lvalue_reference<T>::type operator->() { return *m_p; }

		bool operator==(const ptr_only<T> & other) const noexcept {
			return this->m_p == other.m_p;
		}

	protected:
		T * m_p;

};

class ptr_only_init_error_init_again : public std::exception {
	public:
		virtual const char* what() const noexcept;
		static void throw_self(); ///< throws this object. Useful to stop copilers from warning that this will result in terminate() when in noexcept
};


/// non-owning poiner, that starts at nullptr and after that can be one time "init'ed" - set to given address. Setting it more then once to ANY address (even to nullptr) results in terminate() or exception
template <typename T>
class ptr_only_init {
	public:
		ptr_only_init(T * p=nullptr)
			: m_p(p) { }

		ptr_only_init<T> & operator=(const ptr_only_init<T> & other) noexcept {
			if (this->m_p != nullptr) ptr_only_init_error_init_again::throw_self();
			this->m_p = other.m_p;
			return *this;
		}


		typename std::add_lvalue_reference<T>::type operator*() { return *m_p; }
		typename std::add_lvalue_reference<T>::type operator->() { return *m_p; }

		bool operator==(const ptr_only_init<T> & other) const noexcept {
			return this->m_p == other.m_p;
		}

	protected:
		T * m_p;
};

} // namespace

#endif


