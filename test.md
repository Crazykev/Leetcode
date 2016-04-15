## 3.1 Docker背后的内核知识
当谈论Docker时，常常会聊到Docker的实现方式。很多开发者都知道，Docker容器本质上是宿主机上的进程。Docker通过namespace实现了资源隔离，通过cgroups实现了资源限制，通过写时复制机制（copy-on-write）实现了高效的文件操作。但当更进一步深入namespace和cgroups等技术细节时，大部分开发者都会感到茫然无措。尤其是在本书的3.4节专门解释libcontainer的工作原理时，我们会大量接触到这部分容器核心知识。所以在这里，希望先带领大家走进Linux内核，了解namespace和cgroups的技术细节。

### 3.1.1 namespace资源隔离

Docker大热之后，热衷技术的开发者会这样思考，想要实现一个资源隔离的容器，应该从哪些方面下手？也许第一反应就是`chroot`命令，这条命令给用户最直观的感受就是使用后根目录`/`的挂载点切换了，即文件系统被隔离了。接着，为了在分布式的环境下进行通信和定位，容器必然要独立的IP、端口、路由等，自然就联想到了网络的隔离。同时，容器还需要一个独立的主机名以便在网络中标识自己。有了网络，自然离不开通信，也就想到了进程间通信需要隔离。开发者可能也已经想到了权限的问题，对用户和用户组的隔离就实现了用户权限的隔离。最后，运行在容器中的应用需要有进程号（`PID`），自然也需要与宿主机中的`PID`进行隔离。

由此，基本上完成了一个容器所需要做的6项隔离，Linux内核中提供了这6种namespace隔离的系统调用，如表3-1所示。当然，真正的容器还需要处理许多其他工作。

表3-1 namespace的6项隔离

namespace |  系统调用参数   |   隔离内容
--------- |    -------    |   --------
UTS       | `CLONE_NEWUTS`  |  主机名与域名
IPC       | `CLONE_NEWIPC`  |  信号量、消息队列和共享内存
PID       | `CLONE_NEWPID`  |  进程编号
Network   | `CLONE_NEWNET`  |  网络设备、网络栈、端口等
Mount     | `CLONE_NEWNS`   |  挂载点（文件系统）
User      | `CLONE_NEWUSER` |  用户和用户组


实际上，Linux内核实现namespace的主要目的之一就是实现轻量级虚拟化（容器）服务。在同一个namespace下的进程可以感知彼此的变化，而对外界的进程一无所知。这样就可以让容器中的进程产生错觉，仿佛自己置身于一个独立的系统环境中，以达到独立和隔离的目的。

需要说明的是，本节所讨论的namespace实现针对的均是Linux内核3.8及以后的版本{![主要原因是user namespace在内核3.8版本以后才支持。]}。接下来，将首先介绍使用namespace的API，然后对这6种namespace进行逐一讲解，并通过程序让读者切身感受隔离效果{![参考自http://lwn.net/Articles/531114/。]}。

#### **1. 进行namespace API操作的4种方式**
namespace的API包括`clone()`、`setns()`以及`unshare()`，还有`/proc`下的部分文件。为了确定隔离的到底是哪6项namespace，在使用这些API时，通常需要指定以下6个参数的一个或多个，通过`|（位或）`操作来实现。读者可能已经在表3-1中注意到，这6个参数分别是`CLONE_NEWIPC`、`CLONE_NEWNS`、`CLONE_NEWNET`、`CLONE_NEWPID`、 `CLONE_NEWUSER`和`CLONE_NEWUTS`。

##### **(1) 通过`clone()`在创建新进程的同时创建namespace**

使用`clone()`来创建一个独立namespace的进程是最常见的做法，也是Docker使用namespace最基本的方法，它的调用方式如下。

```
int clone(int (*child_func)(void *), void *child_stack, int flags, void *arg);
```

`clone()`实际上是Linux系统调用`fork()`的一种更通用的实现方式，它可以通过`flags`来控制使用多少功能。一共有20多种`CLONE_*`的`flag`（标志位）参数用来控制`clone`进程的方方面面（如是否与父进程共享虚拟内存等），下面挑选与namespace相关的4个参数进行说明。

* `child_func`传入子进程运行的程序主函数。
* `child_stack`传入子进程使用的栈空间。
* `flags`表示使用哪些`CLONE_*`标志位，与namespace相关的主要包括`CLONE_NEWIPC`、`CLONE_NEWNS`、`CLONE_NEWNET`、`CLONE_NEWPID`、 `CLONE_NEWUSER`和`CLONE_NEWUTS`。
* `args`则可用于传入用户参数。

在后续内容中将会有使用`clone()`的实际程序供大家参考。

##### **(2) 查看/proc/[pid]/ns文件**

从3.8版本的内核开始，用户就可以在`/proc/[pid]/ns`文件下看到指向不同namespace号的文件，效果如下所示，形如`[4026531839]`者即为namespace号。

```
$ ls -l /proc/$$/ns         <<-- $$是shell中表示当前运行的进程ID号
total 0
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 ipc -> ipc:[4026531839]
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 mnt -> mnt:[4026531840]
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 net -> net:[4026531956]
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 pid -> pid:[4026531836]
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 user->user:[4026531837]
lrwxrwxrwx. 1 mtk mtk 0 Jan  8 04:12 uts -> uts:[4026531838]
```

如果两个进程指向的namespace编号相同，就说明它们在同一个namespace下，否则便在不同namespace里面。`/proc/[pid]/ns`里设置这些link的的另外一个作用是，一旦上述link文件被打开，只要打开的文件描述符（`fd`）存在，那么就算该namespace下的所有进程都已经结束，这个namespace也会一直存在，后续进程也可以再加入进来。在Docker中，通过文件描述符定位和加入一个存在的namespace是最基本的方式。

另外，把`/proc/[pid]/ns`目录文件使用`--bind`方式挂载起来可以起到同样的作用，命令如下：

```
# touch ~/uts
# mount --bind /proc/27514/ns/uts ~/uts
```
为了方便起见，后面的讲解中会使用这个`~/uts`文件来代替` /proc/27514/ns/uts`。

> {注意} 如果读者看到ns下的内容与本节所述不符，那可能是因为使用了3.8以前版本的内核。如在内核版本2.6中，该目录下存在的只有`ipc`、`net`和`uts`，并且以硬链接方式存在。

##### **(3) 通过`setns()`加入一个已经存在的namespace**
上文提到，在进程都结束的情况下，也可以通过挂载的形式把namespace保留下来，保留namespace的目的是为以后有进程加入做准备。在Docker中，使用`docker exec`命令在已经运行着的容器中执行一个新的命令，就需要用到该方法。通过`setns()`系统调用，进程从原先的namespace加入某个已经存在的namespace，使用方法如下。通常为了不影响进程的调用者，也为了使新加入的pid namespace生效{![在pid namespace一节会具体解释。]}，会在`setns()`函数执行后使用`clone()`创建子进程继续执行命令，让原先的进程结束运行。

```
int setns(int fd, int nstype);
```

* 参数`fd`表示要加入namespace的文件描述符。上文提到，它是一个指向`/proc/[pid]/ns`目录的文件描述符，可以通过直接打开该目录下的链接或者打开一个挂载了该目录下链接的文件得到。
* 参数`nstype`让调用者可以检查`fd`指向的namespace类型是否符合实际要求。该参数为`0`表示不检查。


为了把新加入的namespace利用起来，需要引入`execve()`系列函数，该函数可以执行用户命令，最常用的就是调用`/bin/bash`并接受参数，运行起一个`shell`，用法如下。

```
fd = open(argv[1], O_RDONLY);   /* 获取namespace文件描述符 */
setns(fd, 0);                   /* 加入新的namespace */
execvp(argv[2], &argv[2]);      /* 执行程序 */
```

假设编译后的程序名称为`setns-test`。

```
# ./setns-test ~/uts /bin/bash   # ~/uts 是绑定的/proc/27514/ns/uts
```

至此，就可以在新加入的namespace中执行`shell`命令了，下文会多次使用这种方式来演示隔离的效果。


##### **(4) 通过`unshare()`在原先进程上进行namespace隔离**

最后要说明的系统调用是`unshare()`，它与`clone()`很像，不同的是，`unshare()`运行在原先的进程上，不需要启动一个新进程。

```
int unshare(int flags);
```

调用`unshare()`的主要作用就是不启动一个新进程就可以起到隔离的效果，相当于跳出原先的namespace进行操作。这样，就可以在原进程进行一些需要隔离的操作。Linux中自带的`unshare`命令，就是通过`unshare()`系统调用实现的。Docker目前并没有使用这个系统调用，这里不做展开，读者可以自行查阅资料学习该命令的知识。

##### (5) `fork()`系统调用
系统调用函数`fork()`并不属于namespace的API，这部分内容属于延伸阅读，如果读者已经对`fork()`有足够多的了解，可以忽略该部分。

当程序调用`fork()`函数时，系统会创建新的进程，为其分配资源，例如存储数据和代码的空间，然后把原来进程的所有值都复制到新进程中，只有少量数值与原来的进程值不同，相当于复制了本身。那么程序的后续代码逻辑要如何区分自己是新进程还是父进程呢？

`fork()`的神奇之处在于它仅仅被调用一次，却能够返回两次（父进程与子进程各返回一次），通过返回值的不同就可以区分父进程与子进程。它可能有以下3种不同的返回值：

* 在父进程中，`fork()`返回新创建子进程的进程ID；
* 在子进程中，`fork()`返回0；
* 如果出现错误，`fork()`返回一个负值。

下面给出一段实例代码，命名为`fork_example.c`。
```
#include <unistd.h>
#include <stdio.h>
int main (){
	pid_t fpid; // fpid表示fork函数返回的值
	int count=0;
	fpid=fork();
	if (fpid < 0)printf("error in fork!");
	else if (fpid == 0) {
		printf("I am child. Process id is %d\n",getpid());
	}
	else {
		printf("i am parent. Process id is %d\n",getpid());
	}
	return 0;
}
```
编译并执行，结果如下。
```
root@local:~# gcc -Wall fork_example.c && ./a.out
I am parent. Process id is 28365
I am child. Process id is 28366
```

代码执行过程中，在语句`fpid=fork()`之前，只有一个进程在执行这段代码，在这条语句之后，就变成父进程和子进程同时执行了。这两个进程的几乎完全相同，将要执行的下一条语句都是`if(fpid<0)`，同时`fpid=fork()`的返回值会依据所属进程返回不同的值。

使用`fork()`后，父进程有义务监控子进程的运行状态，并在子进程退出后自己才能正常退出，否则子进程就会成为“孤儿”进程。

下面将根据Docker内部对namespace资源隔离使用的方式分别对6种namespace进行详细的解析。

#### **2. UTS namespace**

UTS（UNIX Time-sharing System）namespace提供了主机名和域名的隔离，这样每个Docker容器就可以拥有独立的主机名和域名了，在网络上可以被视作一个独立的节点，而非宿主机上的一个进程。Docker中，每个镜像基本都以自身所提供的服务名称来命名镜像的hostname，且不会对宿主机产生任何影响，其原理就是利用了UTS namespace。

下面通过代码来感受一下UTS隔离的效果，首先需要一个程序的骨架。打开编辑器创建`uts.c`文件，输入如下代码。
```
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];
char* const child_args[] = {
  "/bin/bash",
  NULL
};

int child_main(void* args) {
  printf("在子进程中!\n");
  execv(child_args[0], child_args);
  return 1;
}

int main() {
  printf("程序开始: \n");
  int child_pid = clone(child_main, child_stack + STACK_SIZE, SIGCHLD, NULL);
  waitpid(child_pid, NULL, 0);
  printf("已退出\n");
  return 0;
}
```

编译并运行上述代码，执行如下命令，效果如下。

```
root@local:~# gcc -Wall uts.c -o uts.o && ./uts.o
程序开始:
在子进程中!
root@local:~# exit
exit
已退出
root@local:~#
```

下面，将修改代码，加入UTS隔离。运行代码需要root权限，以防止普通用户任意修改系统主机名导致`set-user-ID`相关的应用运行出错。

```
//[...]
int child_main(void* arg) {
  printf("在子进程中!\n");
  sethostname("NewNamespace", 12);
  execv(child_args[0], child_args);
  return 1;
}

int main() {
//[...]
int child_pid = clone(child_main, child_stack+STACK_SIZE,
    CLONE_NEWUTS | SIGCHLD, NULL);
//[...]
}
```

再次运行，可以看到hostname已经变化。
```
root@local:~# gcc -Wall namespace.c -o main.o && ./main.o
程序开始:
在子进程中!
root@NewNamespace:~# exit
exit
已退出
root@local:~#  <- 回到原来的hostname
```

值得一提的是，也许有读者会尝试不加`CLONE_NEWUTS`参数运行上述代码，发现主机名同样改变了，并且输入`exit`后主机名也恢复了，似乎并没有区别。实际上，不加`CLONE_NEWUTS`参数进行隔离时，由于使用`sethostname`函数，所以宿主机的主机名被修改了。而看到`exit`退出后主机名还原，是因为bash只在刚登录时读取一次UTS，不会实时读取最新的主机名。当重新登录或者使用`uname`命令进行查看时，就会发现产生的变化。

#### **3. IPC namespace**

进程间通信（Inter-Process Communication，IPC）涉及的IPC资源包括常见的信号量、消息队列和共享内存。申请IPC资源就申请了一个全局唯一的32位ID，所以IPC namespace中实际上包含了系统IPC标识符以及实现POSIX消息队列的文件系统。在同一个IPC namespace下的进程彼此可见，不同IPC namespace下的进程则互相不可见。

IPC namespace在实现代码上与UTS namespace相似，只是标识位有所变化，需要加上`CLONE_NEWIPC`参数。主要改动如下，其他部分不变，程序名称改为`ipc.c`{![测试方法参考自：http://crosbymichael.com/creating-containers-part-1.html。]}。

```
//[...]
int child_pid = clone(child_main, child_stack+STACK_SIZE,
           CLONE_NEWIPC | CLONE_NEWUTS | SIGCHLD, NULL);
//[...]
```

首先在shell中使用`ipcmk -Q`命令创建一个message queue。
```
root@local:~# ipcmk -Q
Message queue id: 32769
```
通过`ipcs -q`可以查看到已经开启的message queue，序号为`32769`。
```
root@local:~# ipcs -q
------ Message Queues --------
key        msqid   owner   perms   used-bytes   messages
0x4cf5e29f 32769   root    644     0            0
```
然后可以编译运行加入了IPC namespace隔离的`ipc.c`，在新建的子进程中调用的`shell`中执行`ipcs -q`查看message queue。

```
root@local:~# gcc -Wall ipc.c -o ipc.o && ./ipc.o
程序开始:
在子进程中!
root@NewNamespace:~# ipcs -q
------ Message Queues --------
key   msqid   owner   perms   used-bytes   messages
root@NewNamespace:~# exit
exit
已退出
```
从结果显示中可以发现，子进程找不到原先声明的message queue了，已经实现了IPC的隔离。

目前使用IPC namespace机制的系统不多，其中比较有名的有PostgreSQL。Docker当前也使用IPC namespace实现了容器与宿主机、容器与容器之间的IPC隔离。

#### **4. PID namespace**

PID namespace隔离非常实用，它对进程PID重新标号，即两个不同namespace下的进程可以有相同的PID。每个PID namespace都有自己的计数程序。内核为所有的PID namespace维护了一个树状结构，最顶层的是系统初始时创建的，被称为root namespace。它创建的新PID namespace被称为child namespace（树的子节点），而原先的PID namespace就是新创建的PID namespace的parent namespace（树的父节点）。通过这种方式，不同的PID namespaces会形成一个层级体系。所属的父节点可以看到子节点中的进程，并可以通过信号等方式对子节点中的进程产生影响。反过来，子节点却不能看到父节点PID namespace中的任何内容，由此产生如下结论{![部分内容引自：http://blog.dotcloud.com/under-the-hood-linux-kernels-on-dotcloud-part。]}。

* 每个PID namespace中的第一个进程“PID 1”，都会像传统Linux中的`init`进程一样拥有特权，起特殊作用。
* 一个namespace中的进程，不可能通过`kill`或`ptrace`影响父节点或者兄弟节点中的进程，因为其他节点的PID在这个namespace中没有任何意义。
* 如果你在新的PID namespace中重新挂载`/proc`文件系统，会发现其下只显示同属一个PID namespace中的其他进程。
* 在root namespace中可以看到所有的进程，并且递归包含所有子节点中的进程。

到这里，读者可能已经联想到一种在外部监控Docker中运行程序的方法了，就是监控Docker daemon所在的PID namespace下的所有进程及其子进程，再进行筛选即可。

下面通过运行代码来感受一下PID namespace的隔离效果。修改上文的代码，加入PID namespace的标识位，并把程序命名为`pid.c`。
```
//[...]
int child_pid = clone(child_main, child_stack+STACK_SIZE,
           CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWUTS 
           | SIGCHLD, NULL);
//[...]
```
编译运行可以看到如下结果。

```
root@local:~# gcc -Wall pid.c -o pid.o && ./pid.o
程序开始:
在子进程中!
root@NewNamespace:~# echo $$
1                      <<--注意此处shell的PID变成了1
root@NewNamespace:~# exit
exit
已退出
```

打印`$$`可以看到shell的PID，退出后如果再次执行可以看到效果如下。
```
root@local:~# echo $$
17542
```
已经回到了正常状态。有的读者可能在子进程的shell中执行了`ps aux`/`top`之类的命令，发现还是可以看到所有父进程的PID，那是因为还没有对文件系统挂载点进行隔离，`ps`/`top`之类的命令调用的是真实系统下的`/proc`文件内容，看到的自然是所有的进程。所以，与其他的namespace不同的是，为了实现一个稳定安全的容器，PID namespace还需要进行一些额外的工作才能确保进程运行顺利，下面将逐一介绍。

##### (1) PID namespace中的`init`进程

在传统的Unix系统中，PID为`1`的进程是`init`，地位非常特殊。它作为所有进程的父进程，维护一张进程表，不断检查进程的状态，一旦有某个子进程因为父进程错误成为了“孤儿”进程，`init`就会负责收养这个子进程并最终回收资源，结束进程。所以在要实现的容器中，启动的第一个进程也需要实现类似`init`的功能，维护所有后续启动进程的运行状态。

当系统中存在树状嵌套结构的PID namespace时，若某个子进程成为孤儿进程，收养该子进程的责任就交给了该子进程所属的PID namespace中的`init`进程。

至此，可能读者已经明白了内核设计的良苦用心。PID namespace维护这样一个树状结构，有利于系统的资源监控与回收。因此，如果确实需要在一个Docker容器中运行多个进程，最先启动的命令进程应该是具有资源监控与回收等管理能力的，如`bash`。

##### (2) 信号与`init`进程
内核还为PID namespace中的`init`进程赋予了其他特权——信号屏蔽。如果`init`中没有编写处理某个信号的代码逻辑，那么与`init`在同一个PID namespace下的进程（即使有超级权限）发送给它的该信号都会被屏蔽。这个功能的主要作用是防止`init`进程被误杀。

那么，父节点PID namespace中的进程发送同样的信号给子节点中的`init`进程，这会被忽略吗？父节点中的进程发送的信号，如果不是`SIGKILL（销毁进程）`或`SIGSTOP（暂停进程）`也会被忽略。但如果发送`SIGKILL`或`SIGSTOP`，子节点的`init`会强制执行（无法通过代码捕捉进行特殊处理），也即是说父节点中的进程有权终止子节点中的进程。

一旦`init`进程被销毁，同一PID namespace中的其他进程也随之接收到`SIGKILL`信号而被销毁。理论上，该PID namespace也不复存在了。但是如果`/proc/[pid]/ns/pid`处于被挂载或者打开状态，namespace就会被保留下来。然而，保留下来的namespace无法通过`setns()`或者`fork()`创建进程，所以实际上并没有什么作用。

当一个容器内存在多个进程时，容器内的`init`进程可以对信号进行捕获，当`SIGTERM`或`SIGINT`等信号到来时，对其子进程做信息保存、资源回收等处理工作。在Docker daemon的源码中也可以看到类似的处理方式，当结束信号来临时，结束容器进程并回收相应资源。

##### (3) 挂载`proc`文件系统

前文提到，如果在新的PID namespace中使用`ps`命令查看，看到的还是所有的进程，因为与PID直接相关的`/proc`文件系统（`procfs`）没有挂载到一个与原`/proc`不同的位置。如果只想看到PID namespace本身应该看到的进程，需要重新挂载`/proc`，命令如下。

```
root@NewNamespace:~# mount -t proc proc /proc
root@NewNamespace:~# ps a
  PID TTY      STAT   TIME COMMAND
    1 pts/1    S      0:00 /bin/bash
   12 pts/1    R+     0:00 ps a
```

可以看到实际的PID namespace就只有两个进程在运行。

> {注意}此时并没有进行mount namespace的隔离，所以该操作实际上已经影响了root namespace的文件系统。当退出新建的PID namespace以后，再执行`ps a`时，就会发现出错，再次执行`mount -t proc proc /proc`可以修复错误。后面还会介绍通过mount namespace来隔离文件系统，当我们基于mount namespace实现了容器`proc`文件系统隔离后，我们就能在Docker容器中使用`ps`等命令看到与PID namespace对应的进程列表。

##### (4) `unshare()`和`setns()`

本章开头就谈到了`unshare()`和`setns()`这两个API，在PID namespace中使用时，也有一些特别之处需要注意。

`unshare()`允许用户在原有进程中建立命名空间进行隔离。但创建了PID namespace后，原先`unshare()`调用者进程并不进入新的PID namespace，接下来创建的子进程才会进入新的namespace，这个子进程也就随之成为新namespace中的`init`进程。

类似地，调用`setns()`创建新PID namespace时，调用者进程也不进入新的PID namespace，而是随后创建的子进程进入。

为什么创建其他namespace时`unshare()`和`setns()`会直接进入新的namespace，而唯独PID namespace例外呢？因为调用`getpid()`函数得到的PID是根据调用者所在的PID namespace而决定返回哪个PID，进入新的PID namespace会导致PID产生变化。而对用户态的程序和库函数来说，它们都认为进程的PID是一个常量，PID的变化会引起这些进程崩溃。

换句话说，**一旦程序进程创建以后，那么它的PID namespace的关系就确定下来了，进程不会变更它们对应的PID namespace**。在Docker中，`docker exec`会使用`setns()`函数加入已经存在的命名空间，但是最终还是会调用`clone()`函数，原因就在于此。

#### **5.   mount namespace**

mount namespace通过隔离文件系统挂载点对隔离文件系统提供支持，它是历史上第一个Linux namespace，所以标识位比较特殊，就是`CLONE_NEWNS`。隔离后，不同mount namespace中的文件结构发生变化也互不影响。可以通过`/proc/[pid]/mounts`查看到所有挂载在当前namespace中的文件系统，还可以通过`/proc/[pid]/mountstats`看到mount namespace中文件设备的统计信息，包括挂载文件的名字、文件系统类型、挂载位置等。

进程在创建mount namespace时，会把当前的文件结构复制给新的namespace。新namespace中的所有`mount`操作都只影响自身的文件系统，对外界不会产生任何影响。这种做法非常严格地实现了隔离，但对某些情况可能并不适用。比如父节点namespace中的进程挂载了一张CD-ROM，这时子节点namespace复制的目录结构是无法自动挂载上这张CD-ROM的，因为这种操作会影响到父节点的文件系统。

2006年引入的**挂载传播（mount propagation）**解决了这个问题，挂载传播定义了**挂载对象（mount object）**之间的关系，这样的关系包括共享关系和从属关系，系统用这些关系决定任何挂载对象中的挂载事件如何传播到其他挂载对象{![参考自：http://www.ibm.com/developerworks/library/l-mount-namespaces/。]}。


* **共享关系（share relationship）。**如果两个挂载对象具有共享关系，那么一个挂载对象中的挂载事件会传播到另一个挂载对象，反之亦然。
* **从属关系（slave relationship）。**如果两个挂载对象形成从属关系，那么一个挂载对象中的挂载事件会传播到另一个挂载对象，但是反之不行；在这种关系中，从属对象是事件的接收者。

一个挂载状态可能为以下一种：

* 共享挂载（shared）
* 从属挂载（slave）
* 共享/从属挂载（shared and slave）
* 私有挂载（private）
* 不可绑定挂载（unbindable）

传播事件的挂载对象称为**共享挂载**；接收传播事件的挂载对象称为**从属挂载**；同时兼有前述两者特征的挂载对象称为**共享/从属挂载**；既不传播也不接收传播事件的挂载对象称为**私有挂载**；另一种特殊的挂载对象称为**不可绑定的挂载**，它们与私有挂载相似，但是不允许执行绑定挂载，即创建mount namespace时这块文件对象不可被复制。通过图3-1可以更好地了解它们的状态变化。

![mount各类挂载状态示意图](/api/storage/getbykey/screenshow?key=150698aaf454588efbf3)

图3-1 mount各类挂载状态示意图

下面我们以图3-1为例说明常用的挂载传播方式。最上层的mount namespace下的/bin目录与child namespace通过master slave方式进行挂载传播，当mount namespace中的/bin目录发生变化时，发生的挂载事件能够自动传播到child namespace中；/lib目录使用完全的共享挂载传播，各namespace之间发生的变化都会互相影响；/proc目录使用私有挂载传播的方式，各mount namespace之间互相隔离；最后的/root目录一般都是管理员所有，不能让其他mount namespace挂载绑定。

默认情况下，所有挂载状态都是私有的。设置为共享挂载的命令如下。

```
mount --make-shared <mount-object>
```
从共享挂载状态的挂载对象克隆的挂载对象，其状态也是共享，它们相互传播挂载事件。

设置为从属挂载的命令如下。
```
mount --make-slave <shared-mount-object>
```
来源于从属挂载对象克隆的挂载对象也是从属的挂载，它也从属于原来的从属挂载的主挂载对象。

将一个从属挂载对象设置为共享/从属挂载，可以执行如下命令，或者将其移动到一个共享挂载对象下。
```
mount --make-shared <slave-mount-object>
```	

如果想把修改过的挂载对象重新标记为私有的，可以执行如下命令。
```
mount --make-private <mount-object>
```

通过执行以下命令，可以将挂载对象标记为不可绑定的。
```
mount --make-unbindable <mount-object>
```

这些设置都可以递归式地应用到所有子目录中，如果读者感兴趣可以自行搜索相关命令。

在代码中实现mount namespace隔离与其他namespace类似，加上`CLONE_NEWNS`标识位即可。让我们再次修改代码，并且另存为`mount.c`进行编译运行。

```
//[...]
int child_pid = clone(child_main, child_stack+STACK_SIZE,
           CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWIPC 
           | CLONE_NEWUTS | SIGCHLD, NULL);
//[...]
```
`CLONE_NEWNS`生效之后，子进程进行的挂载与卸载操作都将只作用于这个mount namespace，因此在上文中提到的处于单独PID namespace隔离中的进程在加上mount namespace的隔离之后，即使该进程重新挂载了`/proc`文件系统，当进程退出后，root mount namespace（主机）的`/proc`文件系统是不会被破坏的。

#### **6.  network namespace**

当我们兴致勃勃地利用前文所讲的各类namespace构建出一个容器，并在容器中启动一个Apache进程时，却出现了“80端口已被占用”的错误，原来主机上已经运行了一个Apache进程，这时就需要借助network namespace技术进行网络隔离。

network namespace主要提供了关于网络资源的隔离，包括网络设备、IPv4和IPv6协议栈、IP路由表、防火墙、`/proc/net`目录、`/sys/class/net`目录、套接字（socket）等。一个物理的网络设备最多存在于一个network namespace中，可以通过创建veth pair（虚拟网络设备对：有两端，类似管道，如果数据从一端传入另一端也能接收到，反之亦然）在不同的network namespace间创建通道，以达到通信目的。
 
一般情况下，物理网络设备都分配在最初的root namespace（表示系统默认的namespace）中。但是如果有多块物理网卡，也可以把其中一块或多块分配给新创建的network namespace。需要注意的是，当新创建的network namespace被释放时（所有内部的进程都终止并且namespace文件没有被挂载或打开），在这个namespace中的物理网卡会返回到root namespace，而非创建该进程的父进程所在的network namespace。

当说到network namespace时，指的未必是真正的网络隔离，而是把网络独立出来，给外部用户一种透明的感觉，仿佛在与一个独立网络实体进行通信。为了达到该目的，容器的经典做法就是创建一个veth pair，一端放置在新的namespace中，通常命名为`eth0`，一端放在原先的namespace中连接物理网络设备，再通过把多个设备接入网桥或者进行路由转发，来实现通信的目的。

也许读者会好奇，在建立起veth pair之前，新旧namespace该如何通信呢？答案是`pipe`（管道）。以Docker daemon启动容器的过程为例，假设容器内初始化的进程称为`init`。Docker daemon在宿主机上负责创建这个`veth pair`，把一端绑定到`docker0`网桥上，另一端接入新建的network namespace进程中。这个过程执行期间，Docker daemon和`init`就通过`pipe`进行通信。具体来说，就是在Docker daemon完成`veth pair`的创建之前，`init`在管道的另一端循环等待，直到管道另一端传来Docker daemon关于`veth`设备的信息，并关闭管道。`init`才结束等待的过程，并把它的“`eth0`”启动起来。整个结构如图3-2所示。

![图像说明文字](/api/storage/getbykey/screenshow?key=1504e94bd1d42d85bdd5)

图3-2 Docker网络示意图

与其他namespace类似，对network namespace的使用其实就是在创建的时候添加`CLONE_NEWNET`标识位。3.8节将会详细介绍Docker的网络，此处不再赘述。

#### **7. user namespaces** 

user namespace主要隔离了安全相关的标识符（identifiers）和属性（attributes），包括用户ID、用户组ID、root目录、[key](http://man7.org/linux/man-pages/man2/keyctl.2.html)（指密钥）以及[特殊权限](http://man7.org/linux/man-pages/man7/capabilities.7.html)。说得通俗一点，一个普通用户的进程通过`clone()`创建的新进程在新user namespace中可以拥有不同的用户和用户组。这意味着一个进程在容器外属于一个没有特权的普通用户，但是它创建的容器进程却属于拥有所有权限的超级用户，这个技术为容器提供了极大的自由。

user namespace是目前的6个namespace中最后一个支持的，并且直到Linux内核3.8版本的时候还未完全实现（还有部分文件系统不支持）。user namespace实际上并不算完全成熟，很多发行版担心安全问题，在编译内核的时候并未开启`USER_NS`。实际上目前Docker也还不支持user namespace，但预留了相应接口，相信在不久后就会支持这一特性。在进行接下来的代码实验时，请确保系统的Linux内核版本高于3.8并且内核编译时开启了`USER_NS`（如果不会选择，请使用Ubuntu14.04）。

Linux中，特权用户的user ID就是0，演示的最后将看到user ID非0的进程启动user namespace后user ID可以变为0。使用user namespace的方法跟别的namespace相同，即调用`clone()`或`unshare()`时加入`CLONE_NEWUSER`标识位。修改代码并另存为`userns.c`，为了看到用户[权限（Capabilities）](http://man7.org/linux/man-pages/man7/capabilities.7.html)，还需要安装`libcap-dev`包。

首先包含以下头文件以调用`Capabilities`包。
```
#include <sys/capability.h>
```
其次在子进程函数中加入`geteuid()`和`getegid()`得到namespace内部的user ID，通过`cap_get_proc()`得到当前进程的用户拥有的权限，并通过`cap_to_text()`输出。
```
int child_main(void* args) {
        printf("在子进程中!\n");
        cap_t caps;
        printf("eUID = %ld;  eGID = %ld;  ",
                        (long) geteuid(), (long) getegid());
        caps = cap_get_proc();
        printf("capabilities: %s\n", cap_to_text(caps, NULL));
        execv(child_args[0], child_args);
        return 1;
}
```
在主函数的`clone()`调用中加入我们熟悉的标识符。
```
//[...]
int child_pid = clone(child_main, child_stack+STACK_SIZE,
            CLONE_NEWUSER | SIGCHLD, NULL);
//[...]
```

至此，第一部分的代码修改就结束了。在编译之前先查看一下当前用户的`uid`和`guid`，请注意此时显示的是普通用户。
```
$ id -u
1000
$ id -g
1000
```

然后开始编译运行，并进入新建的user namespace，会发现shell提示符前的用户名已经变为`nobody`。


```
$ gcc userns.c -Wall -lcap -o userns.o && ./userns.o
程序开始：
在子进程中!
eUID = 65534;  eGID = 65534;  capabilities: = cap_chown,cap_dac_override,[...]37+ep  <<--此处省略部分输出，已拥有全部权限
nobody@ubuntu$ 
```

通过验证可以得到以下信息。

* user namespace被创建后，第一个进程被赋予了该namespace中的全部权限，这样该`init`进程就可以完成所有必要的初始化工作，而不会因权限不足出现错误。
* 从namespace内部观察到的UID和GID已经与外部不同了，默认显示为65534，表示尚未与外部namespace用户映射。此时需要对user namespace内部的这个初始user和它外部namespace的某个用户建立映射，这样可以保证当涉及一些对外部namespace的操作时，系统可以检验其权限（比如发送一个信号量或操作某个文件）。同样用户组也要建立映射。
* 还有一点虽然不能从输出中发现，但却值得注意。用户在新namespace中有全部权限，但它在创建它的父namespace中不含任何权限，就算调用和创建它的进程有全部权限也是如此。因此哪怕是root用户调用了`clone()`在user namespace中创建出的新用户，在外部也没有任何权限。
* 最后，user namespace的创建其实是一个层层嵌套的树状结构。最上层的根节点就是root namespace，新创建的每个user namespace都有一个父节点user namespace，以及零个或多个子节点user namespace，这一点与PID namespace非常相似。

从图3-3中可以看到，namespace实际上就是按层次关联起来，每个namespace都发源于最初的root namespace并与之建立映射。

![图像说明文字](/api/storage/getbykey/screenshow?key=15038403507d773d83eb)

图3-3 namespace映射图{![图片来源：《深入Linux内核架构》第2章命名空间。]}


接下来就要进行用户绑定操作，通过在` /proc/[pid]/uid_map`和`/proc/[pid]/gid_map`两个文件中写入对应的绑定信息就可以实现这一点，格式如下。

```
ID-inside-ns   ID-outside-ns   length
```

写这两个文件时需要注意以下几点。

* 这两个文件只允许由拥有该user namespace中`CAP_SETUID`权限的进程写入一次，不允许修改。
* 写入的进程必须是该user namespace的父namespace或者子namespace。
* 第一个字段`ID-inside-ns`表示新建的user namespace中对应的user/group ID，第二个字段`ID-outside-ns`表示namespace外部映射的user/group ID。最后一个字段表示映射范围，通常填1，表示只映射一个，如果填大于1的值，则按顺序建立一一映射。

明白了上述原理，再次修改代码，添加设置`uid`和`gid`的函数。

```
//[...]
void set_uid_map(pid_t pid, int inside_id, int outside_id, int length) {
    char path[256];
    sprintf(path, "/proc/%d/uid_map", getpid());
    FILE* uid_map = fopen(path, "w");
    fprintf(uid_map, "%d %d %d", inside_id, outside_id, length);
    fclose(uid_map);
}
void set_gid_map(pid_t pid, int inside_id, int outside_id, int length) {
    char path[256];
    sprintf(path, "/proc/%d/gid_map", getpid());
    FILE* gid_map = fopen(path, "w");
    fprintf(gid_map, "%d %d %d", inside_id, outside_id, length);
    fclose(gid_map);
}
int child_main(void* args) {
	cap_t caps;
	printf("在子进程中!\n");
	set_uid_map(getpid(), 0, 1000, 1);
	set_gid_map(getpid(), 0, 1000, 1);
	printf("eUID = %ld;  eGID = %ld;  ",
			(long) geteuid(), (long) getegid());
	caps = cap_get_proc();
	printf("capabilities: %s\n", cap_to_text(caps, NULL));
	execv(child_args[0], child_args);
	return 1;
}
//[...]
```

编译后即可看到user已经变成了`root`。

```
$ gcc userns.c -Wall -lcap -o usernc.o && ./usernc.o
程序开始:
在子进程中!
eUID = 0;  eGID = 0;  capabilities: = [...],37+ep
root@ubuntu:~#
```

至此，就已经完成了绑定的工作，可以看到演示全程都是在普通用户下执行的，最终实现了在user namespace中成为root用户，对应到外部则是一个uid为1000的普通用户。

如果要把user namespace与其他namespace混合使用，那么依旧需要root权限。解决方案是先以普通用户身份创建user namespace，然后在新建的namespace中作为root，在`clone()`进程加入其他类型的namespace隔离。

讲解完user namespace，再来谈谈Docker。Docker目前尚未使用user namespace，但是使用了在user namespace中涉及的Capabilities机制。从内核2.2版本开始，Linux把原来和超级用户相关的高级权限划分为不同的单元，称为Capability。这样管理员就可以独立对特定的Capability进行使能或禁止。Docker虽然没有使用user namespace，但可以禁用容器中不需要的Capability，以便在一定程度上加强容器安全性。

说到安全，namespace的6项隔离看似全面，实际上依旧没有完全隔离Linux的资源，比如`SELinux`、 `cgroups`以及`/sys`、`/proc/sys`、`/dev/sd* `等目录下的资源。关于安全，将会在3.9节中进一步探讨。


 本节从namespace使用的API开始，结合Docker逐步对6个namespace进行了讲解。相信把讲解过程中所有的代码整合起来，读者也能实现一个属于自己的“shell”容器了。虽然namespace技术使用非常简单，但要真正把容器做到安全易用却并非易事。PID namespace中，需要实现一个完善的`init`进程来维护好所有进程；network namespace中，还有复杂的路由表和iptables规则没有配置；user namespace中还有许多权限问题需要考虑。其中的某些方面Docker已经做得不错，而有些方面才刚刚起步，这些内容我们会在3.4节libcontainer原理中详细介绍。


### 3.1.2 cgroups资源限制

上一节中，我们了解了Docker背后使用的资源隔离技术namespace，通过系统调用构建一个相对隔离的shell环境，也可以称之为简单的“容器”。这一节，将讲解另一个强大的内核工具——cgroups。它不仅可以限制被namespace隔离起来的资源，还可以为资源设置权重、计算使用量、操控任务（进程或线程）启停等。在本节最后介绍完基本概念后，将详细讲解Docker中使用到的cgroups内容。

> {注意}在Linux系统中，内核本身的调度和管理并不对进程和线程进行区分，只根据`clone`创建时传入参数的不同，来从概念上区别进程和线程，所以本节统一称之为任务。

#### **1. cgroups是什么**

cgroups最初名为process container，由Google工程师Paul Menage和Rohit Seth于2006年提出，后来由于container有多重含义容易引起误解，就在2007年更名为control groups，并整合进Linux内核，顾名思义就是把任务放到一个组里面统一加以控制。官方的定义如下{![引自：https://www.kernel.org/doc/Documentation/cgroups/cgroups.txt。]}：

> cgroups是Linux内核提供的一种机制，这种机制可以根据需求把一系列系统任务及其子任务整合（或分隔）到按资源划分等级的不同组内，从而为系统资源管理提供一个统一的框架。

通俗地来说，cgroups可以限制、记录任务组所使用的物理资源（包括CPU、memory、IO等），为容器实现虚拟化提供了基本保证，是构建Docker等一系列虚拟化管理工具的基石。

对开发者来说，cgroups有如下4个特点。

* cgroups的API以一个伪文件系统的方式实现，用户态的程序可以通过文件操作实现cgroups的组织管理。
* cgroups的组织管理操作单元可以细粒度到线程级别，另外用户可以创建和销毁cgroup，从而实现资源再分配和管理。
* 所有资源管理的功能都以子系统的方式实现，接口统一。
* 子任务创建之初与其父任务处于同一个cgroups的控制组。

本质上来说，cgroups是内核附加在程序上的一系列钩子（hook），通过程序运行时对资源的调度触发相应的钩子以达到资源追踪和限制的目的。


#### **2. cgroups的作用**

实现cgroups的主要目的是为不同用户层面的资源管理，提供一个统一化的接口。从单个任务的资源控制到操作系统层面的虚拟化，cgroups提供了以下四大功能{![参考自：http://en.wikipedia.org/wiki/cgroups。]}。

* 资源限制：cgroups可以对任务使用的资源总额进行限制。如设定应用运行时使用内存的上限，一旦超过这个配额就发出OOM（Out of Memory）提示。
* 优先级分配：通过分配的CPU时间片数量及磁盘IO带宽大小，实际上就相当于控制了任务运行的优先级。
* 资源统计： cgroups可以统计系统的资源使用量，如CPU使用时长、内存用量等，这个功能非常适用于计费。
* 任务控制：cgroups可以对任务执行挂起、恢复等操作。

过去有一段时间，内核开发者甚至把namespace也作为一个cgroups的子系统加入进来，也就是说cgroups曾经甚至还包含了资源隔离的能力。但是资源隔离会给cgroups带来许多问题，如pid namespace加入后，PID在循环出现的时候cgroup会出现命名冲突、cgroup创建后进入新的namespace导致其他子系统资源脱离了控制等{![详见：https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=a77aea92010acf54ad785047234418d5d68772e2。]}，所以在2011年就被移除了。


#### **3. cgroups术语表**

* **task（任务）**：在cgroups的术语中，任务表示系统的一个进程或线程。
* **cgroup（控制组）**：cgroups中的资源控制都以cgroup为单位实现。cgroup表示按某种资源控制标准划分而成的任务组，包含一个或多个子系统。一个任务可以加入某个cgroup，也可以从某个cgroup迁移到另外一个cgroup。
* **subsystem（子系统）**：cgroups中的子系统就是一个资源调度控制器。比如CPU子系统可以控制CPU时间分配，内存子系统可以限制cgroup内存使用量。
* **hierarchy（层级）**：层级由一系列cgroup以一个树状结构排列而成，每个层级通过绑定对应的子系统进行资源控制。层级中的cgroup节点可以包含零或多个子节点，子节点继承父节点挂载的子系统。整个操作系统可以有多个层级。

#### **4. 组织结构与基本规则**

在3.1.1节已经介绍过，传统的Unix任务管理，实际上是先启动`init`任务作为根节点，再由`init`节点创建子任务作为子节点，而每个子节点又可以创建新的子节点，如此往复，形成一个树状结构。而系统中的多个cgroup也构成类似的树状结构，子节点从父节点继承属性。

它们最大的不同在于，系统中的多个cgroup构成的层级并非单根结构，可以允许存在多个。如果任务模型是由`init`作为根节点构成的一棵树，那么系统中的多个cgroup则是由多个层级构成的森林。这样做的目的很好理解，如果只有一个层级，那么所有的任务都将被迫绑定其上的所有子系统，会给某些任务造成不必要的限制。在Docker中，每个子系统独自构成一个层级，这样做非常易于管理。

了解了cgroups的组织结构，再来了解cgroups、任务、子系统、层级四者间的关系及其基本规则{![参考自：https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/sec-Relationships_Between_Subsystems_Hierarchies_Control_Groups_and_Tasks.html。]}。

* **规则1：** 同一个层级可以附加一个或多个子系统。如图3-4所示，CPU和Memory的子系统附加到了一个层级。

![图像说明文字](/api/storage/getbykey/screenshow?key=1603850f0878e7250246)

图3-4 同一个层级可以附加一个或多个子系统

* **规则2：**  一个子系统可以附加到多个层级，当且仅当目标层级只有唯一一个子系统时。图3-5中小圈中的数字表示子系统附加的时间顺序，CPU子系统附加到层级A的同时不能再附加到层级B，因为层级B已经附加了内存子系统。如果层级B没有附加过内存子系统，那么CPU子系统同时附加到两个层级是允许的。

![图像说明文字](/api/storage/getbykey/screenshow?key=16035f42996676596e5f)

图3-5 一个已经附加在某个层级上的子系统不能附加到其他含有别的子系统的层级上

* **规则3：** 系统每次新建一个层级时，该系统上的所有任务默认加入这个新建层级的初始化cgroup，这个cgroup也被称为root cgroup。对于创建的每个层级，任务只能存在于其中一个cgroup中，即一个任务不能存在于同一个层级的不同cgroup中，但一个任务可以存在于不同层级中的多个cgroup中。如果操作时把一个任务添加到同一个层级中的另一个cgroup中，则会将它从第一个cgroup中移除。在图3-6中可以看到，`httpd`任务已经加入到层级A中的`/cg1`，而不能加入同一个层级中的`/cg2`，但是可以加入层级B中的`/cg3`。

![图像说明文字](/api/storage/getbykey/screenshow?key=16039fd7270c6b3a4a33)

图3-6 一个任务不能属于同一个层级的不同cgroup

* **规则4：** 任务在fork/clone自身时创建的子任务默认与原任务在同一个cgroup中，但是子任务允许被移动到不同的cgroup中。即fork/clone完成后，父子任务间在cgroup方面是互不影响的。图3-7中小圈中的数字表示任务出现的时间顺序，当`httpd`刚fork出另一个`httpd`时，两者在同一个层级中的同一个cgroup中。但是随后如果ID为4840的`httpd`需要移动到其他cgroup也是可以的，因为父子任务间已经独立。总结起来就是：初始化时子任务与父任务在同一个cgroup，但是这种关系随后可以改变。

![图像说明文字](/api/storage/getbykey/screenshow?key=16039356b08c37990b3a)

图3-7 刚fork/clone出的子任务在初始状态与其父任务处于同一个cgroup


#### **5. 子系统简介**

子系统实际上就是cgroups的资源控制系统，每种子系统独立地控制一种资源，目前Docker使用如下9种子系统，还有一种`net_cls`子系统在内核中已经广泛实现，但是Docker尚未采用，Docker在网络方面的控制方式在3.8节有详细介绍，以下是它们的用途。

* **blkio：** 可以为块设备设定输入/输出限制，比如物理驱动设备（包括磁盘、固态硬盘、USB等）。
* **cpu：** 使用调度程序控制任务对CPU的使用。
* **cpuacct：** 自动生成cgroup中任务对CPU资源使用情况的报告。
* **cpuset：** 可以为cgroup中的任务分配独立的CPU（此处针对多处理器系统）和内存。
* **devices：** 可以开启或关闭cgroup中任务对设备的访问。
* **freezer：** 可以挂起或恢复cgroup中的任务。
* **memory：** 可以设定cgroup中任务对内存使用量的限定，并且自动生成这些任务对内存资源使用情况的报告。
* **perf_event：**使用后使cgroup中的任务可以进行统一的性能测试。{![Perf: Linux CPU性能探测器，详见https://perf.wiki.kernel.org/index.php/Main_Page。]}
* ***net_cls：** Docker没有直接使用它，它通过使用等级识别符（classid）标记网络数据包，从而允许Linux流量控制程序（Traffic Controller，TC）识别从具体cgroup中生成的数据包。


上述cgroup子系统如何使用虽然很重要，但是Docker并没有对cgroup本身做增强，容器用户一般也不需要直接操作cgroup，所以本书会在附录D、E中再为读者详细地讲解上述子系统的完整内容和使用方法，这里我们只大致说明一下操作流程，让读者有一个感性的认识。

Linux中cgroup实现形式表现为一个文件系统，因此需要mount这个文件系统才能够使用（也有可能已经mount好了），挂载成功后，就能看到各类子系统。

```
#Ubuntu 14.04
$ mount -t cgroup
cgroup on /sys/fs/cgroup/cpuset type cgroup (rw,relatime,cpuset)
cgroup on /sys/fs/cgroup/cpu type cgroup (rw,relatime,cpu)
cgroup on /sys/fs/cgroup/cpuacct type cgroup (rw,relatime,cpuacct)
cgroup on /sys/fs/cgroup/memory type cgroup (rw,relatime,memory)
cgroup on /sys/fs/cgroup/devices type cgroup (rw,relatime,devices)
cgroup on /sys/fs/cgroup/freezer type cgroup (rw,relatime,freezer)
cgroup on /sys/fs/cgroup/blkio type cgroup (rw,relatime,blkio)
cgroup on /sys/fs/cgroup/net_cls type cgroup (rw,net_cls)
cgroup on /sys/fs/cgroup/perf_event type cgroup (rw,relatime,perf_event)
```
以cpu子系统为例，先看一下挂载了这个子系统的控制组下的文件。
```
$ ls /sys/fs/cgroup/cpu
cgroup.clone_children  cgroup.procs          cpu.cfs_period_us  cpu.rt_period_us   cpu.shares            release_agent
cgroup.sane_behavior  cpu.cfs_quota_us   cpu.rt_runtime_us  cpu.stat    notify_on_release  tasks
```

在/sys/fs/cgroup的cpu子目录下创建控制组，控制组目录创建成后，它下面就会有很多类似的文件了。

```
# in /sys/fs/cgroup/cpu
$ sudo mkdir cg1
$ ls cg1
cgroup.clone_children  cgroup.procs  cpu.cfs_period_us  cpu.cfs_quota_us  cpu.rt_period_us  cpu.rt_runtime_us  cpu.shares  cpu.stat  notify_on_release  tasks
```
下面的例子展示了如何限制PID为18828的进程的cpu使用配额：
```
# 限制18828进程
$ echo 18828 >> /sys/fs/cgroup/cpu/cg1/tasks
# 将cpu限制为最高使用20%
$ echo 20000 > /sys/fs/cgroup/cpu/cg1/cpu.cfs_quota_us
```

在Docker的实现中，Docker daemon会在单独挂载了每一个子系统的控制组目录（比如/sys/fs/cgroup/cpu）下创建一个名为`docker`的控制组，然后在`docker`控制组里面，再为每个容器创建一个以容器ID为名称的容器控制组，这个容器里的所有进程的进程号都会写到该控制组`tasks`中，并且在控制文件（比如cpu.cfs_quota_us）中写入预设的限制参数值。综上，`docker`组的层级结构如下。
```
$ tree cgroup/cpu/docker
cgroup/cpu/docker/
├── 0e8220dbfeac5cb96eb34c4b1a0d648e9358f205bec8c803bc1f7fc178cb8f78 # 这是容器ID
│   ├── cgroup.clone_children
│   ├── cgroup.procs
│   ├── cpu.cfs_period_us
│   ├── cpu.cfs_quota_us #这里有值
│   ├── cpu.rt_period_us
│   ├── cpu.rt_runtime_us
│   ├── cpu.shares
│   ├── cpu.stat
│   ├── notify_on_release
│   └── tasks
├── cgroup.clone_children
├── cgroup.procs
├── cpu.cfs_period_us
├── cpu.cfs_quota_us
├── cpu.rt_period_us
├── cpu.rt_runtime_us
├── cpu.shares
├── cpu.stat
├── notify_on_release
└── tasks
```
关于Docker如何实现cgroup的配置的问题，将会在3.4节libcontainer里做解释。

#### **6. cgroups实现方式及工作原理简介**

在对cgroups规则和子系统有了一定了解以后，下面简单介绍操作系统内核级别上cgroups的工作原理，希望能有助于读者理解cgroups如何对Docker容器中的进程产生作用。

cgroups的实现本质上是给任务挂上**钩子**，当任务运行的过程中涉及某种资源时，就会触发钩子上所附带的子系统进行**检测**，根据资源类别的不同使用对应的技术进行资源限制和优先级分配。

#####cgroups如何判断资源超限及超出限额之后的措施

对于不同的系统资源，cgroups提供了统一的接口对资源进行控制和统计，但限制的具体方式则不尽相同。比如memory子系统，会在描述内存状态的“mm_struct”结构体中记录它所属的cgroup，当进程需要申请更多内存时，就会触发cgroup用量检测，用量超过cgroup规定的限额，则拒绝用户的内存申请，否则就给予相应内存并在cgroup的统计信息中记录。实际实现要比以上描述复杂得多，不仅需考虑内存的分配与回收，还需考虑不同类型的内存如Cache（缓存）和Swap（交换区内存拓展）等。

进程所需的内存超过cgroup最大限额以后，如果设置了OOM Control（内存超限控制），那么进程就会收到OOM信号并结束；否则进程就会被挂起，进入睡眠状态，直到cgroup中其他进程释放了足够的内存资源为止。Docker中默认是开启OOM Control的。其他子系统的实现与此类似，cgroups提供了多种资源限制的策略供用户选择。

#####cgroup的子系统与任务之间的关联关系

实现上，cgroup与任务之间是多对多的关系，所以它们并不直接关联，而是通过一个中间结构把双向的关联信息记录起来。每个任务结构体`task_struct`中都包含了一个指针，可以查询到对应cgroup的情况，同时也可以查询到各个子系统的状态，这些子系统状态中也包含了找到任务的指针，不同类型的子系统按需定义本身的控制信息结构体，最终在自定义的结构体中把子系统状态指针包含进去，然后内核通过`container_of`（这个宏可以通过一个结构体的成员找到结构体自身）等宏定义来获取对应的结构体，关联到任务，以此达到资源限制的目的。同时，为了让cgroups便于用户理解和使用，也为了用精简的内核代码为cgroup提供熟悉的权限和命名空间管理，内核开发者们按照Linux 虚拟文件系统转换器（Virtual Filesystem Switch，VFS）接口实现了一套名为cgroup的文件系统，非常巧妙地用来表示cgroups的层级概念，把各个子系统的实现都封装到文件系统的各项操作中。有兴趣的读者可以阅读VFS{![参见http://en.wikipedia.org/wiki/Virtual_file_system。]}的相关内容，在此就不赘述了。

##### Docker在使用cgroup时的注意事项

在实际的使用过程中，Docker需要通过挂载cgroup文件系统新建一个层级结构，挂载时指定要绑定的子系统。把cgroup文件系统挂载上以后，就可以像操作文件一样对cgroups的层级进行浏览和操作管理（包括权限管理、子文件管理等）。除了cgroup文件系统以外，内核没有为cgroups的访问和操作添加任何系统调用。

如果新建的层级结构要绑定的子系统与目前已经存在的层级结构完全相同，那么新的挂载会重用原来已经存在的那一套（指向相同的css_set）。否则，如果要绑定的子系统已经被别的层级绑定，就会返回挂载失败的错误。如果一切顺利，挂载完成后层级就被激活并与相应子系统关联起来，可以开始使用了。

目前无法将一个新的子系统绑定到激活的层级上，或者从一个激活的层级中解除某个子系统的绑定。

当一个顶层的cgroup文件系统被卸载（umount）时，如果其中创建过深层次的后代cgroup目录，那么就算上层的cgroup被卸载了，层级也是激活状态，其后代cgroup中的配置依旧有效。只有递归式地卸载层级中的所有cgroup，那个层级才会被真正删除。


在创建的层级中创建文件夹，就类似于fork了一个后代cgroup，后代cgroup中默认继承原有cgroup中的配置属性，但是可以根据需求对配置参数进行调整。这样就把一个大的cgroup系统分割成一个个嵌套的、可动态变化的“软分区”。

#####`/sys/fs/cgroup/cpu/docker/<container-ID>`下文件的作用

前面已经说过，以资源开头（比如cpu.shares）的文件都是用来限制这个cgroup下任务的可用的配置文件。一个cgroup创建完成，不管绑定了何种子系统，其目录下都会生成以下几个文件，用来描述cgroup的相应信息。同样，把相应信息写入这些配置文件就可以生效，内容如下。

一个cgroup创建完成，不管绑定了何种子系统，其目录下都会生成以下几个文件，用来描述cgroup的相应信息。同样，把相应信息写入这些配置文件就可以生效，内容如下。

 - `tasks`：这个文件中罗列了所有在该cgroup中任务的TID，即所有进程或线程的ID。该文件并不保证任务的TID有序，把一个任务的TID写到这个文件中就意味着把这个任务加入这个cgroup中，如果这个任务所在的任务组与其不在同一个cgroup，那么会在cgroup.procs文件里记录一个该任务所在任务组的TGID值，但是该任务组的其他任务并不受影响。
 - `cgroup.procs`：这个文件罗列所有在该cgroup中的TGID（线程组ID），即线程组中第一个进程的PID。该文件并不保证TGID有序和无重复。写一个TGID到这个文件就意味着把与其相关的线程都加到这个cgroup中。
 - `notify_on_release`：填0或1，表示是否在cgroup中最后一个任务退出时通知运行`release agent`，默认情况下是0，表示不运行。
 - `release_agent`：指定release agent执行脚本的文件路径（该文件在最顶层cgroup目录中存在），这个脚本通常用于自动化卸载无用的cgroup。

本节由浅入深地讲解了cgroups，从cgroups是什么，到cgroups该怎么用，最后对大量的cgroup子系统配置参数进行了梳理。可以看到，内核对cgroups的支持已经较多，但是依旧有许多工作需要完善。如网络方面目前通过TC（Traffic Controller）来控制，未来需要统一整合；优先级调度方面依旧有很大的改进空间。

