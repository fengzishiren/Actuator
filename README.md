类shell脚本解释器
===
我之前一直不知道把这个解释器做成什么样子，一个原因是我对linux shell没任何兴趣。但是我觉得这是一个方向，朝着这个方向作，搞不好某一天就取代linux shell了。

**这是一个简单的脚本执行引擎，最终目标：**
> * 1.精准的报错能力（具体到某一个行的某一列）
> * 2.简化语法（尽量使用霍夫曼编码使得常用指令尽可能的短）
> * 4.加速
> * 5.编写扩展
> * 6.未完待续。。。

目前支持的命令（每一行都是一条指令或命令，第一个单词为指令名称，后面为参数）：
* exit 
* goto 
* say 
* print 
* read 
* if 
* shell 
注意：say和print命令是一样的 都会自动输出字符串并换行，是从Perl6借鉴来的，我本人很喜欢。

如果你创建一个变量的话尽量不要与以上命令重名，也就是传说中的保留字，但是如果重名了也没什么，具体原因可以看源代码。

#怎么用？
示例代码片段（这里面read命令的后面跟的shell是一个变量，我可以让它和sehll命令重名，可以看出它仍然可以正常工作。）：
```perl
L1:
	say "Please enter shell command:"
	read shell
	if shell == "quit" goto L2
	shell shell
	goto L1
L2:
	say "bye-bye"

```
这段脚本用到了上面所有的命令，注意L1和L2有点编程经验的人都知道是做什么用的。需要注意的是不要同名否则最后的label会覆盖掉前面同名的label。（完整的文件在example下shell.se中）


以下是其他代码示例：

print代码示例1：
```python
print "Ode to the West Wind"
print "——Percy·Bysshe·Shelley"
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
执行结果：  
Ode to the West Wind  
——Percy·Bysshe·Shelley  
O wild West Wind, thou breath of Autumn's being,  
Thou, from whose unseen presence the leaves dead  
Are driven, like ghosts from an enchanter fleeing,  
Yellow, and black, and pale, and hectic red,  
Pestilence-stricken multitudes: O thou,  
Who chariotest to their dark wintry bed  
The winged seeds, where they lie cold and low,  
Each like a corpse within its grave, until  
Thine azure sister of the Spring shall blow  
Her clarion o'er the dreaming earth, and fill  
(Driving sweet buds like flocks to feed in air)  
With living hues and odours plain and hill:  
Wild Spirit, which art moving everywhere;  
Destroyer and preserver; hear, oh hear!  

Echo代码示例2：  
``` python
START:
	print "Entry a string Or (quit)"
	read val
	if val == "quit" goto FINISH
	print val
	goto START
FINISH:
	exit
```
执行结果:  
Entry a string Or (quit)  
hello,world!  
hello,world!  
Entry a string Or (quit)  
Actuator.............  
Actuator.............  
Entry a string Or (quit)  
bye-bye  
bye-bye    
Entry a string Or (quit)  
quit  


