脚本解释器
===

===
###build
编译器：c++11
cmake: 见CMakeLists.txt

解释器用法：
```sh
lunatic@lunatic:~/devcenter/workspace/Actuator$ ./debug/hello.out -h
usage: ./debug/hello.out [options] [filenames]
  -name           read file named name(omit this argument)
  -d <level>      set the logging level
  -v              show version information
  --version       show version information
  -h              help information
  --help          help information
```


---------------------

**这是一个简单的脚本执行引擎，最终目标：**
> * 1.精准的报错能力（具体到某一个行的某一列）
> * 2.简化语法（尽量使用霍夫曼编码使得常用指令尽可能的短）
> * 4.加速
> * 5.编写扩展
> * 6.未完待续。。。


**命名规则：**

和c语言一样以下划线'_'或字母开头，后接数字、字母、下划线和'-'的组合。关于'-'是从lisp语言借鉴而来的。
合法的标示符（变量名）：aaa, _, _a, _2, _----, get-set等。

**注意事项：**

* say和print命令是一样的 都会自动输出字符串并换行，是从Perl6借鉴来的，我本人很喜欢。
* lisp注释风格 ';'作用于';'出现的位置到当前行尾

如果你创建一个变量的话尽量不要与以上命令重名，也就是传说中的保留字，但是如果重名了也没什么，具体原因可以看源代码。

**怎么用？** 

print代码示例（雪莱的西风颂）：
```python
print "Ode to the West Wind"
print "                     ——Percy·Bysshe·Shelley"
print "O wild West Wind, thou breath of Autumn's being,"
print "Thou, from whose unseen presence the leaves dead"
print "Are driven, like ghosts from an enchanter fleeing,"
print "Yellow, and black, and pale, and hectic red,"
print "Pestilence-stricken multitudes: O thou,"
print "Who chariotest to their dark wintry bed"
print "The winged seeds, where they lie cold and low,"
print "Each like a corpse within its grave, until"
print "Thine azure sister of the Spring shall blow"
print "Her clarion o'er the dreaming earth, and fill"
print "(Driving sweet buds like flocks to feed in air)"
print "With living hues and odours plain and hill:"
print "Wild Spirit, which art moving everywhere;"
print "Destroyer and preserver; hear, oh hear!"
```
示例代码片段（斐波那契数列）：
```python
def fib(n)
    a = 0
    b = 1
    i = 0
    loop i < n
        t = b
        add b a
        a = t
        add i 1
    end
    say a
end
fib(100)
```

更多代码示例参见examples目录　

