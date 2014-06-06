
**我们的目的是重写这门语言的执行引擎，最终目标：**
> * 1.精准的报错能力（具体到某一个行的某一列）
> * 2.简化语法（尽量使用霍夫曼编码使得常用指令尽可能的短）
> * 4.加速
> * 5.编写扩展
> * 6.未完待续。。。


新语法：
print "输入个数字"
read num
if num < 2 goto FAIL
if num == 2 goto SUCC
mov eax 2

L:
  if num == eax goto SUCCESS
  mod edx num eax
  if edx == 0 goto FAIL
  add eax eax 1
  goto L

SUC:
  print "OK!"
FAIL:
  print num "不是质数。"
  
### 语言设计


原文地址：http://www.cppblog.com/vczh/archive/2008/07/10/55793.html

这门语言使用如下语法：
 
> * <值>：值可以是整数、小数、字符串或名字。

> * <名>：名可以是变量名或者标号等，使用字母与下划线开始，后接不定数量的字母、下划线与数字。

> * <名>:：名字后接冒号代表一个标号。这个标号代表着一个指令的位置，用于指定跳转目标。

> * goto <名>：goto用于直接跳转到一个位置继续执行。

> * set <名> <值>：set用于将一个值赋值给一个指定名字的变量。这个变量不存在则创建。

> * opcode <名> <值> <值>：opcode可以是add、minus、mul、div、idiv或mod。这6个命令将两个值进行加、减、乘、除、整除 及求余，并将结果赋值给一个指定名字的变量。这个变量不存在则创建。

> * if <值>[ opcode <值>] goto <名>：if用于判断一个条件并在条件满足被满足的时候跳转到指定的地方。条件可以是一个值，这个值必须是整数，并且在这个值不为0的时候条件被满足。条件也可以是一个比较，这个时候opcode可以是is、is_not、less_than、greater_than、less_equal或greater_equal，分别在第一个值等于、不等于、小于、大于、小于或等于、大于或等于第二个值的时候满足条件。

> * exit：结束执行

> * <名> <值>*：如果命令名称不是上面的5种的其中一种的话，那么这个命令将被传递给插件进行执行。这个时候，命令可以有任意的参数。
 
在这种语法下，我们可以假设宿主程序给了我们write、writeln和read命令用于输入输出，并得到一个判断输入的数字是否质数的程序：
```
  write "请输入一个数字："
  read Number
  if Number less_then 2 goto FAIL
  if Number is 2 goto SUCCESS
  set Divisor 2
LOOP_BEGIN:
  if Number is Divisor goto SUCCESS
  mod Remainder Number Divisor
  if Remainder is 0 goto FAIL
  add Divisor Divisor 1
  goto LOOP_BEGIN
SUCCESS:
  writeln Number "是质数。"
  exit
FAIL:
  writeln Number "不是质数。"
```
这个程序首先判断输入是不是小于等于2，如果不是的话则使用一种简单的方法来判断输入是不是质数。
假设输入的数字为n，那么在n>2的时候，如果2到n-1中的任何一个数字能够整除n的话，那么n就不是质数了。


[原始地址][2]

[1]: http://www.cppblog.com/vczh/archive/2008/07/10/55793.html
[2]: http://www.cppblog.com/vczh/archive/2008/07/10/55793.html
