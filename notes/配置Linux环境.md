# 配置Linux环境

1. 使用root用户登录，注意：输入密码的时候没有回显
2. 执行`apt install sudo`命令，安装sudo软件包。
3. 执行`usermod -G sudo 用户名`命令，将普通用户添加到sudo用户组，这样普通用户就可以使用sudo，执行一些只有管理员（root用户）可以执行的命令。

# 远程登录

下载PuTTY：https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe

查询虚拟机的IP地址：`ip addr`

![image-20200630092947347](%E9%85%8D%E7%BD%AELinux%E7%8E%AF%E5%A2%83.assets/image-20200630092947347.png)

打开PuTTY登录到虚拟机：

![image-20200630093055865](%E9%85%8D%E7%BD%AELinux%E7%8E%AF%E5%A2%83.assets/image-20200630093055865.png)

注意：默认不能使用root用户远程登录，必须使用普通用户。

# 远程文件拷贝

下载winscp：https://winscp.net/download/WinSCP-5.17.6-Portable.zip

解压缩zip文件，执行winscp.exe。

![image-20200630094153744](%E9%85%8D%E7%BD%AELinux%E7%8E%AF%E5%A2%83.assets/image-20200630094153744.png)

输入虚拟机IP，用户名和密码，点击save按钮。

![image-20200630094253886](%E9%85%8D%E7%BD%AELinux%E7%8E%AF%E5%A2%83.assets/image-20200630094253886.png)

点击`save password`保存密码，点击OK。

点击`Login`按钮连接到虚拟机。