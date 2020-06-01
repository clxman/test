# SFINAE　
 - 原文的链接   
       [SFINAE](https://time.geekbang.org/column/article/181636)　　

  
## C++模板实例化和调用
 -  这里面引入一个模板(函数模板和类模板)实例化和调用过程的文章不错
   [c++模板实例化与调用]（https://blog.csdn.net/K346K346/article/details/49490185）  

  - 模板实例化  
    - 隐形模板实例化

        不存在指定参数的的模板实例（函数模板或类模模板），编译器根据函数参数去隐式实例化函数模板或类模板的过程　就是隐式实例化   
        - 函数模板隐式实例化　  　
           １.  一般过程参加下面的f<>函数的实例化过程  
           ２.  间接实例化:一个函数指针，被赋值时，实际函数名字如果不是一个函数，那么编译器就会搜索可见范围内的同名模板，尝试实例化。
      - 类模板隐式实例化  
           １.　代码中直接使用类模板时　才去实例化模板。例如:
           
  ```bash
  　ClassTest<TypeA> classtest ;
      classtest.print();
  ```
  - 显示实例化  
        在不使用函数模板或类模板的情况下，实例化函数模板或类模板　
         - 函数模板和类模板实例化
```bash
  　　template [函数返回类型] [函数模板名]<实际类型列表>（函数参数列表）
  　　例如:template void func<int>(const int&);
         template class [类模板名]<实际类型列表>
        template class theclass<int>;

```
 - 函数模板的调用方式  
       - 隐式调用：调用时不直接给出模板参数，而是根据参数推导(deduce)过程。寻找并匹配函数模板，并实例化     
       - 显示模板实参调用 :调用时，显示的给出模板参数。不需要编译器进行deduce过程。有时显示模板实参调用是必须的，因为参数推导可能失败　　

```bash　
　#include <iostream>
using namespace std;
template <typename T> T Max(const T& t1,const T& t2)
{
	return (t1>t2)?t1:t2;
}

int main()
{
	int i=5;
	//cout<<Max(i,'a')<<endl; //无法通过编译
	cout<<Max<int>(i,'a')<<endl; //显示调用，通过编译
}
```
## 重载决议  
```bash

#include <stdio.h>

struct Test {
  typedef int foo;
};

template <typename T>
void f(typename T::foo)
{
  puts("1");
}

template <typename T>
void f(T)
{
  puts("2");
}

int main()
{
  f<Test>(10);
  f<int>(10);
}

```

- 首先匹配函数名字
    - f<Test<(10) 有两个函数模板名匹配
- 然后替换模板参数，形成函数定义，所有的模板参数类型实例化过程。实例化函数定义了？
    - Test模板参数：　f<Test>(Test::foo)  f<Test>(Test)  
    - Int模板参数:  f<Int>(Int::foo) :int没有foo子类型，所有不会实例化这个函数定义的，f<Int>(Int) 合理的，形成函数定义
- 用实际参数匹配，看看能不能调用成功（编译器处理一般函数的调用选择方法）

## SFINAE设计初衷
 - SFINAE最初用法：如果模板实例化中发生了失败，没有理由编译就此出错终止，因为还是可能有其他可用的函数重载的。这儿的失败仅指函数模板的原型声明，即参数和返回值
 ## SFINAE的最佳实践
  - enable_if_t:获取enable_if的成员type类型
      - 例如enable_if_t<has_reserve<C>::value, void>，表示如果C类型有成员函数reservere，就是has_reserve<C>::value为true时，enable_if_t 返回void类型，否则实例化失败了。就不会产生函数或类定义
 -  decltype
    - delctype获取一个成员或表达式的类型。
    - declval可以生成某个类型，但是只适合在用于模板匹配（匹配成功返回值类型和值的判断，）不能用于真正的运算过程。代码中的实现不会被执行(unevaluated context)
     
     ```bash
     Converts any type T to a reference type, making it possible to use member functions in decltype expressions without the need to go through constructors.
     these operators only query the compile-time properties of their operands. Thus, std::size_t n = sizeof(std::cout << 42); does not perform console output.
     ```
  
    - 例子使用decltype的含义。
       -decltype后面是逗号表达式，返回值是最后一个参数，这里表达的意思是如果delval成功，则返回void类型
       - 测试用例test_sfinae_example中有使用decltype(declval)的其他小例子
 ```bash
template <typename C, typename T>
auto append(C& container, T* ptr,
            size_t size)
  -> decltype(
    declval<C&>().reserve(1U),
    void())
    
{
  container.reserve(
    container.size() + size);
  for (size_t i = 0; i < size;
       ++i) {
    container.push_back(ptr[i]);
  }
}
 ```
    - 模板特化和SFINAE技术的差距  
    　- 模板特化只是针对某一个类型，ＳＦＩＮＡＥ技术可以针对类型的共同特征，进行switch case类似的分类，像上面的has_reserve是根据类型有没有reserver函数形成不同的实例化定义。
        - 模板特化只是针对模板参数的某个特定类型。
    - tag dispatch实例，可以看一下，作为标签的处理方式，参照例子。　不是不需要参数，是在编译期构造好。传入。
    　- 例子参见代码test_sfina_example.cpp中的tag_dispatch代码
   - 静态检测的限制：必须在编译期确定所有函数的定义，参见用例的描述　   
   - void_t;把任意类型转换为void类型。这个过程编译器会检测模板参数的有效性，有效则返回，否则啥也没实例化，SFINE的过程。例如下面

  ```bash
template <typename...>
using void_t = void;

template <typename T,
          typename = void_t<>>
struct has_reserve : false_type {};

//这个就会失败对于没有reserve成员的函数
template <typename T>
struct has_reserve<
  T, void_t<decltype(
       declval<T&>().reserve(1U))>>
  : true_type {};

  ```