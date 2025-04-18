def fun(x):
       n=0
       t=100
#**********SPACE**********
       while t < 1000:
               s1=t%10
#**********SPACE**********
               s2=(t//10)%10
               s3=t//100
#**********SPACE**********
               if  s1+s2+s3 == x:
                       print(t, end="、")
                       n+=1                        
               t+=1
       print("")
       return   n
def main():
       x = int(input('输入一个正整数:'))
       print("当x值为%d时，100～999之间各位上数字之和为%d的整数分别是：" %(x,x))      
       print("共%d个" %fun(x))     
if __name__ == '__main__':
       main()