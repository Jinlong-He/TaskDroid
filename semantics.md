# Understanding Android Mutitasking Mechanism
## 1 Basic Concepts
In this section, I will introduce some basic concepts in the Android system which is important for understanding Android Mutitasking Mechanism.
### 1.1 Activity
Activity is a crucial component in Android app. Generally an activity implements one screen(user interface).
### 1.2 Task (Back Stack)
A task is a collection of activities that users interact with when performing a certain job, a back stack is uesd to store the activities in the task.

```
Activities in Current Activity Manager State:
  * TaskRecord{44d07218 #4 A android.task.contacts}
    clearOnBackground=true numActivities=2 rootWasReset=true
    affinity=android.task.contacts
    intent={act=android.intent.action.MAIN cat=[android.intent.category.LAUNCHER] flg=0x10600000 cmp=com.android.contacts/.DialtactsActivity bnds=[125,640][235,758]}
    origActivity=com.android.contacts/.DialtactsContactsEntryActivity
    realActivity=com.android.contacts/.DialtactsActivity
    lastActiveTime=288203177 (inactive for 14s)
```

- `realActivity` is an attribute of a task, it values the activity which create this task.
- `affinity` is the [task affinity](#15) of `realActivity`.

### 1.3 Task Stack
A task stack is used to store the back stacks for different tasks.

<h3 id=14>1.4 Launch Mode</h3>

Launch mode is an attribute of Activity in Android system, there are four launch modes could be set as followed. Here we give some high level understanding of these launch modes, more details will be represented in the [semantics part](#21). 

- `standard`: 
    - It is the default value. 
    - Multiple instances can be instantiated and pushed into the same task or different tasks.
    - The instance of Activity will be pushed into the top task directly.

![standard](https://github.com/LoringHe/TaskDroid/blob/master/pictures/standard.png)

- `singleTop`: 
    - The difference from `standard` is if there exists an instance of activity at the top of current task, it will be reused instead of instantiating a new one.

![singleTop](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTop.png)

- `singleTask` 
    - There is only one instance of activity which set this launch mode in one task.
    - The Task affinity of the instance of activity is the task affinity of the task stored it.
    - If there exists a task which task affinity is equal to `Aft(B)` and its realActivity's lauch mode is not `SIT`, then that task will be moved to the top of task stack first, and
        - if B is not in the top task, B will be pushed into the top task directly;
        - otherwise, the top task will pop until B is on the top;
    - otherwise, it will create a new task which realActivity is B, and this task will be pushed into task stack.

![singleTask3](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask3.png)

![singleTask2](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask2.png)

![singleTask1](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)


- `singleInstance`
    - There is only one instance of activity which set this launch mode in the whole task stack.
    - No activities instance can be pushed into the same task of this instance's.
    - If there is no one task which realActivity is B, it will create a new task which realActivity is B, and this task will be pushed into task stack;
    - otherwise, the task which realAcitivity is B will be moved to the top of task stack.

![singleInstance1](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)

![singleInstance2](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleInstance.png)

<h3 id=15>1.5 Task Affinity</h3>

The task affinity of an activity indicates to which task the activity prefers to belong.

``` xml 
    <activity android:launchMode=["standard" | "singleTop" | "singleTask" | "singleInstance"];
              android:taskAffinity="string">
    </activity>
```
We can both set [lauch mode](#14) and [task affinity](#15) of an Activity in the [Manifest.xml](https://developer.android.com/guide/topics/manifest/manifest-intro).

### 1.6 Intent Flag
- FLAG_ACTIVITY_NEW_TASK 
- FLAG_ACTIVITY_CLEAR_TOP 
- FLAG_ACTIVITY_SINGLE_TOP 
- FLAG_ACTIVITY_CLEAR_TASK 
- FLAG_ACTIVITY_MULTIPLE_TASK 
- FLAG_ACTIVITY_TASK_ON_HOME 
- FLAG_ACTIVITY_REORDER_TO_FRONT 

We can set the intent flag in the function `startActivity(Intent intent)`.

``` java
    public class A extends Activity {
        public void onCreate() {
            Intent intent = new Intent(B.class());
            intent.addFlags(intent.FLAG_ACTIVITY_NEW_TASK)
            intent.addFlags(intent.FLAG_ACTIVITY_CLEAR_TOP)
            startActivity(intent);
            }
        }
```
## 2 Task Allocation Mechanism
For simply speaking, we define some notations.
- `Lmd(A)` return the launch mode of the activity A.
- `Aft(A)` return the task affinity of the activity A.
- `Rat(S)` return the realActivity of the task S.
- `Aft(S)` return the task affinity of the task S.

The task allocation mechanism, namely, to specify, when an activity is launched, to which task will it be allocated.
Recall the semantics of launching an activity with `singleTask` or `singleInstance` launch mode, they both need to allocate to a task first. 
In the following, I will give a detailed description about the task allocation mechanism.
There are two kinds of task allocation mechanism.

- 1) SingleInstance Task Allocation Mechanism: 
    - To find the task which `realActivity` is instance of B.
- 2) Non-SingleInstance Task Allocation Mechanism: 
    - To find the first task S which `realActivity` is instance of B.
    - if not founded, to find the first task S which `task affinity` is equal to the `task affinity` of B. 

We define a function `allocateTask(B, TS)` to simulate Non-SingleInstance Task Allocation Mechanism, where `TS` means the current task stack.

```
    Task allocateTask(B, TS) {
        for (Task S from TS.top() to TS.bot())
            if (Rat(S) = B) return S;
        for (Task S from TS.top() to TS.bot())
            if (Aft(S) = Aft(B)) return S;
        return null;
    } 
```

## 3 Operating on Task and Task Stack
In this section I will introduce some operations on task (reps. task stack). 
All the behavious of `startActivity()` are based on the combinations of these operations.
### 3.1 Operating on Task Stack
After understanding the Task Allocation Mechanism, I will introduce two sorts of operations depends on the result of `allocateTask(B, TS)`, 

- MoveTask2Top(S): Move the task S to the top of task stack.
    - Task Allocation Mechanism finds a task S, and S in not the top task of task stack.
- LaunchTask(B): Create a new task which `realActivity` is B and push it into task stack. 
    - Task Allocation Mechanism doesn't find a task.
- TaskOnHome(S): Move all the tasks expect task S.
    - Task stack change.
- TaskOnHome(B): Move all the tasks and LaunchTask(B).
    - Task stack change.
- NoAction().

### 3.2 Operating on Task 
There are four operations on task as followed:

- LaunchAct(B): Push the instance of B into the top task.
    - etc. Launch an standard activity B.
- ClearTop(B): Pop activity until the top of the top task is the instance of B.
    - etc. Launch an singleTask activity B.
- ClearTask(B): Pop activity until the top task is empty, and Push(B).
    - etc. Launch an activity B with intent flag `FLAG_ACTIVITY_CLEAR_TASK`.
- MoveAct2Top(B): Move the first instance of B to the top of the top task.
    - etc. Launch an activity B with intent flag `FLAG_ACTIVITY_REORDER_TO_FRONT`.
- NoAction()

## 4 Deep Understanding Semantics
In this section I will tell you how to decide the behavious of `A.startActivity(B, Fs)`, 
which means A startActivity B with the intent flags Fs.

### 4.1 `Lmd(B) = singleInstance`
#### 4.1.1 Methodology
##### Find a task S via SingleInstance Task Allocation Mechanism.
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack.
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(S)**;
    - otherwise, **MoveTask2Top(S)**.
- if S is not existed.
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(B)**;
    - otherwise, **LaunchTask(B)**.

#### 4.1.2 Expriments

- {Lmd(B) = singleInstance; A = B}
    - **NoAction()**

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.1.png)

- {Lmd(B) = singleInstance; A != B; ~~`FLAG_ACTIVITY_TASK_ON_HOME`~~;} 
    - **MoveTask2Top(S)**
    - **LaunchTask(B)**

![MoveTask2Top(S)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleInstance.png)

![LaunchTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)

- {Lmd(B) = singleInstance; A != B; `FLAG_ACTIVITY_TASK_ON_HOME`;} 
    - **TaskOnHome(S)**
    - **TaskOnHome(B)**

![TaskOnHome(S)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.2.png)

![TaskOnHome(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.4.png)

### 4.2 `Lmd(B) = singleTask`
#### 4.2.1 Methodology
##### a. Find a task S via Non-SingleInstance Task Allocation Mechanism.
##### b. Operate on task stack
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack,
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(S)**;
    - otherwise, **MoveTask2Top(S)**;
- if S is not existed,
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(B)**;
    - otherwise, **LaunchTask(B)**.

##### c. Operate on task 
- if `FLAG_ACTIVITY_CLEAR_TASK` is in Fs, then **ClearTask(B)**;
- otherwise, 
    - if there exists an instance which class is B in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.

#### 4.2.2 Expriments
We let S' as the top task of task stack.
- {Lmd(B) = singleTask; `FLAG_ACTIVITY_CLEAR_TASK`; Aft(B) = Aft(S');}
    - **NoAction()** + **ClearTask(B)**

![NoAction() + ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.1.png)

- {Lmd(B) = singleTask; ~~`FLAG_ACTIVITY_CLEAR_TASK`~~; Aft(B) = Aft(S');}
    - **NoAction()** + **ClearTop(B)**
    - **NoAction()** + **LaunchAct(B)**

![NoAction() + ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.2.png)

![NoAction() + LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

- {Lmd(B) = singleTask; `FLAG_ACTIVITY_CLEAR_TASK` + `FLAG_ACTIVITY_TASK_ON_HOME`; Aft(B) != Aft(S');}
    - **TaskOnHome(S)** + **ClearTask(B)**
    - **TaskOnHome(B)** 

![TaskOnHome(S) + ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.4.png)

![TaskOnHome(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.5.png)

- {Lmd(B) = singleTask; `FLAG_ACTIVITY_CLEAR_TASK`; ~~`FLAG_ACTIVITY_TASK_ON_HOME`~~; Aft(B) != Aft(S');}
    - **MoveTask2Top(S)** + **ClearTask(B)**
    - **LaunchTask(B)** 

![MoveTask2Top(S) + ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.6.png)

![LaunchTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)

- {Lmd(B) = singleTask; `FLAG_ACTIVITY_TASK_ON_HOME`;  ~~`FLAG_ACTIVITY_CLEAR_TASK`~~; Aft(B) != Aft(S');}
    - **TaskOnHome(S)** + **ClearTop(B)**
    - **TaskOnHome(S)** + **LaunchAct(B)**

![TaskOnHome(S) + ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.7.png)

![TaskOnHome(S) + LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.8.png)

- {Lmd(B) = singleTask; ~~`FLAG_ACTIVITY_CLEAR_TASK`~~ + ~~`FLAG_ACTIVITY_TASK_ON_HOME`~~; Aft(B) != Aft(S');}
    - **MoveTask2Top(S)** + **ClearTop(B)**
    - **MoveTask2Top(S)** + **LaunchAct(B)**

![MoveTask2Top(S) + ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.9.png)

![MoveTask2Top(S) + LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.10.png)

### 4.3 `Lmd(B) = standard`
#### 4.3.1 Methodology
##### Decide whether to allocate task.
- if `Lmd(A) = singleInstance` or `FLAG_ACTIVITY_NEW_TASK`, then goto a.
- otherwise, goto d.

##### a. Find a task S via Non-SingleInstance Task Allocation Mechanism.
##### b. Operate on task stack
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack,
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(S)**;
    - otherwise, **MoveTask2Top(S)**;
- if S is not existed,
    - if `FLAG_ACTIVITY_TASK_ON_HOME` is in Fs, then **TaskOnHome(B)**;
    - otherwise, **LaunchTask(B)**.

##### c. Operate on task 
- if `FLAG_ACTIVITY_CLEAR_TASK` is in Fs, then **ClearTask(B)**;
- else if `FLAG_ACTIVITY_CLEAR_TOP` is in Fs,
    - if there exists an instance which class is B in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.
- else if `FLAG_ACTIVITY_SINGLE_TOP` is in Fs 
    - if the class of the top instance in the top task is B, then **NoAction()**;
    - else if B is the class of the `realActivity` of S and B is not `mainActivity`, then **NoAction()**;
    - otherwise, **LaunchAct(B)**.
- else if B is the class of the `realActivity` of S and B is not `mainActivity`, then **NoAction()**;
- otherwise, **LaunchAct(B)**.

##### d. Operate on task 
- if `FLAG_ACTIVITY_CLEAR_TOP` is in Fs,
    - if there exists an instance which class is B in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.
- else if `FLAG_ACTIVITY_REORDER_TO_FRONT` is in Fs,
    - if there exists an instance which class is B in the top task, then **MoveAct2Top(B)**;
    - otherwise, **LaunchAct(B)**.
- else if `FLAG_ACTIVITY_SINGLE_TOP` is in Fs 
    - if the class of the top instance in the top task is B, then **NoAction()**;
    - otherwise, **LaunchAct(B)**.
- otherwise, **LaunchAct(B)**.

#### 4.3.2 Expriments
- {Lmd(B) = standard; Lmd(A) != singleInstance; ; `FLAG_ACTIVITY_CLEAR_TOP`; ~~`FLAG_ACTIVITY_NEW_TASK`~~;}
    - **ClearTop(B)**
    - **LaunchAct(B)**

![ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.2.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

- {Lmd(B) = standard; Lmd(A) != singleInstance; `FLAG_ACTIVITY_REORDER_TO_FRONT`; ~~`FLAG_ACTIVITY_CLEAR_TOP`~~ +  ~~`FLAG_ACTIVITY_NEW_TASK`~~;}
    - **MoveAct2Top(B)**
    - **LaunchAct(B)**

![MoveAct2Top(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.3.1.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

- {Lmd(B) = standard; Lmd(A) != singleInstance; `FLAG_ACTIVITY_SINGLE_TOP`; ~~`FLAG_ACTIVITY_REORDER_TO_FRONT`~~ + ~~`FLAG_ACTIVITY_CLEAR_TOP`~~ +  ~~`FLAG_ACTIVITY_NEW_TASK`~~;}
    - **NoAction()**
    - **LaunchAct(B)**

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.1.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

- {Lmd(B) = standard; Lmd(A) != singleInstance; ~~`FLAG_ACTIVITY_SINGLE_TOP`~~ +  ~~`FLAG_ACTIVITY_REORDER_TO_FRONT`~~ + ~~`FLAG_ACTIVITY_CLEAR_TOP`~~ +  ~~`FLAG_ACTIVITY_NEW_TASK`~~;}
    - **LaunchAct(B)**

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

### 4.4 Principal
#### 4.4.1 Equivalence Rule
##### 1) (Lmd(A) = singleInstance) == `FLAG_ACTIVITY_NEW_TASK`
##### 2) (Lmd(B) = singleTop) == (Lmd(B) = standard) + `FLAG_ACTIVITY_SINGLE_TOP`
##### 3) (Lmd(B) = singleTask) == (Lmd(B) = standard) + `FLAG_ACTIVITY_NEW_TASK` + `FLAG_ACTIVITY_CLEAR_TOP`
#### 4.4.2 Enable Rule
##### 1) `FLAG_ACTIVITY_NEW_TASK` -> `FLAG_ACTIVITY_CLEAR_TASK`
##### 2) `FLAG_ACTIVITY_NEW_TASK` -> `FLAG_ACTIVITY_MULTIPLE_TASK`
##### 3) `FLAG_ACTIVITY_NEW_TASK` -> `FLAG_ACTIVITY_TASK_ON_HOME`
#### 4.4.3 Disable Rule
##### 1) `FLAG_ACTIVITY_NEW_TASK` => `FLAG_ACTIVITY_REORDER_TO_FRONT`
##### 2) `FLAG_ACTIVITY_CLEAR_TOP` => `FLAG_ACTIVITY_REORDER_TO_FRONT`
##### 3) `FLAG_ACTIVITY_CLEAR_TASK` => `FLAG_ACTIVITY_CLEAR_TOP`
##### 4) `FLAG_ACTIVITY_CLEAR_TASK` => `FLAG_ACTIVITY_SINGLE_TOP`
