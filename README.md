
**我们的目的是重写这门语言的执行引擎，最终目标：**
> * 1.精准的报错能力（具体到某一个行的某一列）
> * 2.简化语法（尽量使用霍夫曼编码使得常用指令尽可能的短）
> * 4.加速
> * 5.编写扩展
> * 6.未完待续。。。


新语法:  
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


