#include <iostream>
#include <stdio.h>
#include<list>
#include<set>
#include<utility>

#include<vector>

// an  interesting fact is  that size is one byte for an empty class or struct for address different with different objects
//see reference:http://www.stroustrup.com/bs_faq2.html#sizeof-empty
template <typename T> struct has_reserve {

  struct good {
    char dummy;
  };
  struct bad {
    char dummy[2];
  };
  // template parameter has different type ,
  // here is a function point to  member function  add of U  class with  size_t
  // parameter and void return type
  template <class U, void (U::*)(size_t)> struct SFINAE {};
  template <class U> static good reserve(SFINAE<U, &U::reserve> *);
  //variadic functions , parase parameters through first paramter literally usually
  // instantiate successfully for arbitrary paramters
  template <class U> static bad reserve(...);
  static const bool value = sizeof(reserve<T>(nullptr)) == sizeof(good);
};

template <typename C, typename T>
std::enable_if_t<has_reserve<C>::value,
            void>
append(C& container, T* ptr,
       size_t size)
{
  std::cout<<"I have reserve"<<std::endl;
  container.reserve(
    container.size() + size);
  for (size_t i = 0; i < size;
       ++i) {
    container.push_back(ptr[i]);
  }
}


template <typename C, typename T>
std::enable_if_t<!has_reserve<C>::value,
            void>
append(C& container, T* ptr,
       size_t size)
{
  std::cout<<"I have not reserve"<<std::endl;
  for (size_t i = 0; i < size; ++i) {
    container.push_back(ptr[i]);
  }
}

template <typename C, typename T>
auto append_v1(C& container, T* ptr,
            size_t size)
  -> decltype(
    std::declval<C&>().reserve(1U),
    void())
{
  container.reserve(
    container.size() + size);
  for (size_t i = 0; i < size;
       ++i) {
    container.push_back(ptr[i]);
  }
}

struct test_withre {
  void reserve(size_t) {}
};
struct test_nore {
  void reserve() {}
};

template <typename T,
          typename =std::void_t<>>
struct has_reserve_v1 : std::false_type {};

template <typename T>
struct has_reserve_v1<
  T, std::void_t<decltype(
      std::declval<T&>().reserve(1U))>>
  : std::true_type {};


template <typename T>
void test_tag_dispatch_and_call(T par,
             std::true_type)
{
   std::cout<< "i am in true:" << par <<std::endl;
}

template <typename T>
void test_tag_dispatch_and_call(T par,
             std::false_type)
{
   std::cout<< "i am in false:" << par <<std::endl;
}


//这里想一下，模板特化和SFINAE技术的差距吧。
//这里不是不需要第二个参数的呀，只不过可以通过模板参数，在编译器根据模板参数构造不同的参数实例，来选择不同的函数。
template <typename T>
void test_tag_dispatch(T par)
{
   test_tag_dispatch_and_call(par, std::integral_constant<bool,std::is_integral<T>::value>{});
}

// 没有reserver方法的对象调用时，会报编译错误，因为所有函数必须在编译器确定，这点和python等动态语言不同 
//改成consexpr应该就不会报错。,constexpr　静态检测报错，增加一下ccls.clang.extraArgs解决
template <typename C, typename T>
void append_static(C& container, T* ptr,
            size_t size)
{
 // if (has_reserve<C>::value) {
 //   container.reserve(
 //     container.size() + size);
 // }
  if constexpr (has_reserve<C>::value) {
    container.reserve(
      container.size() + size);
  }
  for (size_t i = 0; i < size;
       ++i) {
    container.push_back(ptr[i]);
  }
}


int main(int argc, char const *argv[]) {
  has_reserve<test_withre> hasr;
  //struct call any member:static or not , use ".""
  std::cout << "has::" << hasr.value << std::endl;
  has_reserve<test_nore> hasn;
  std::cout << "has::" << hasn.value << std::endl;
  std::list<int> list;
  std::vector<int> vec;
  std::set<int> set;
  
  int a = 1;
  //list not have reserve so append   comply with the rule of function template instaninate with SFINAE rule
  // match the second
  append(list,&a,1);
  // match the first 
  append(vec,&a,1);
  //append(set,&a,1);

   // compile fail 
  //append_v1(list,&a,1);
  append_v1(vec,&a,1);
  //通用
 //decltype获取表达式的类型，declval编译态实例化。并调用 
// using typea   =  decltype(std::declval< std::vector<int>>().pop_back());
// void a = nullptr;
auto val= has_reserve_v1<std::vector<int>>::value;
std::cout<< "has reserve v1:" <<val;
int val_int = 1;
double val_double = 1;
test_tag_dispatch(val_int);
test_tag_dispatch(val_double);
append_static(list,&val_int,1);
 return 0;

}
