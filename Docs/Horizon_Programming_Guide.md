## HORIZON-API-12: Create(), Destroy()

Functions for creating objects that allocate the object's memory should be called `create_xxx()`.
The corresponding function for destroying the object should be called `destroy_xxx()`.

If the functions don't allocate memory, but just initialize an object that already exists, they
should be called `init_xxx()` and `shutdown_xxx()` instead.

## 内存分配

维持最低限度的堆分配，且永不再紧凑循环中使用堆分配

## 缓存友好

### 冷数据/热数据分割

有人可能认为这样能最大程度利用CPU缓存：把一个对象所有要用的数据（包括组件数据）都塞进一个类里，而没有任何用指针或引用的形式间接存储数据。

实际上这个想法是错误的，我们不能忽视一个问题：CPU缓存的存储空间是有限的

于是我们希望CPU缓存存储的是经常使用的数据，而不是那些少用的数据。这就引入了冷数据/热数据分割的概念了。

热数据：经常要操作使用的数据，我们一般可以直接作为可直接访问的成员变量。

冷数据：比较少用的数据，我们一般以引用/指针来间接访问（即存储的是指针或者引用）。

### 避免无效数据夹杂在连续内存区域

## #频繁调用的函数尽可能不要做成虚函数

C++的虚函数机制，简单来说是两次地址跳转的函数调用，这对CPU缓存十分不友好，往往命中失败。

实际上虚函数可以优雅解决很多面向对象的问题，然而在游戏程序如果有很多虚函数都要频繁调用（例如每帧调用），很容易引发性能问题。

解决方法是，把这些频繁调用的虚函数尽可能去除virtual特性（即做成普通成员函数），并避免调用基类对象的成员函数，代价是这样一改得改很多与之牵连代码。

所以最好一开始设计程序时，需要先想好哪些最好不要写成virtual函数。