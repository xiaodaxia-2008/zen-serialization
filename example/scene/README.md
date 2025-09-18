# Note
`cereal` 序列化库的功能比较全面，支持多态，但是需要把实现都放到头文件中，不能放到源文件中。可以参考这个例子，在多dll中容易出错。  

如果需要夸dll并且在cpp中实现，需要在调用的dll/exe中添加注册以及集成关系声明：

```cpp
CEREAL_REGISTER_TYPE(Derived);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base, Derived);
```
