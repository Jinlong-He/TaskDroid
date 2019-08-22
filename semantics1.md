# Understanding Android Mutitasking Mechanism
## How to use Launch Mode and Intent flags
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

Launch mode is an attribute of Activity in Android system, there are four launch modes could be set as followed. Here we give some high level understanding of these launch modes, more details will be represented in the later.

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
Intent flags are used with `startActivity(Intent)` via `setFlags(int)` and `addFlags(int)` like the code snippet below:

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

There are many intent flags, today we focus on the flags as followed.
- FLAG_ACTIVITY_NEW_TASK 
- FLAG_ACTIVITY_CLEAR_TOP 
- FLAG_ACTIVITY_SINGLE_TOP 
- FLAG_ACTIVITY_CLEAR_TASK 
- FLAG_ACTIVITY_MULTIPLE_TASK 
- FLAG_ACTIVITY_TASK_ON_HOME 
- FLAG_ACTIVITY_REORDER_TO_FRONT 

## 2 Task Allocation Mechanism

The task allocation mechanism, namely, to specify, when an activity is launched, to which task will it be allocated.
Recall the semantics of launching an activity with `singleTask` or `singleInstance` launch mode, they both need to allocate to a task first. 
In the following, I will give a detailed description about the task allocation mechanism.
There are two kinds of task allocation mechanism.

<h3 id=21>2.1 SingleInstance Task Allocation Mechanism</h3>

Namely, to allocate task for activity with singleInstance launch mode. 

To find the task which the `realActivity`'s class is B,
when the callee activity B set launch mode as `singleInstance`, 

<h3 id=22>2.2 Non-SingleInstance Task Allocation Mechanism</h3>

Namely, to allocate task for activity with non-singleInstance launch mode. 

To find the first task which `realActivity`'s class is B;
if not founded, to find the first task which `task affinity` is the same as `task affinity` of B, 
when 1) the caller activity A set launch mode as `singleInstance` 
or 2) the callee activity B set launch mode as `singleTask` 
or 3) the intent flag `FLAG_ACTIVITY_NEW_TASK` is used, 

## 3 Operations on Task and Task Stack
In this section I will introduce some operations on task (reps. task stack). 
All the behavious of `startActivity()` are based on the combinations over the operations on task and task stack.
### 3.1 Operations on Task Stack
After understanding the Task Allocation Mechanism, I will introduce five sorts of operations depends on the result of `allocateTask(B, TS)`, 

- MoveTask2Top(S): Move the task S to the top of task stack.
    - Task Allocation Mechanism finds a task S, and S in not the top task of task stack.
- LaunchTask(B): Create a new task which `realActivity` is B and push it into task stack. 
    - Task Allocation Mechanism doesn't find a task.
- TaskOnHome(S): Move all the tasks expect task S.
    - Task stack change.
- TaskOnHome(B): Move all the tasks and LaunchTask(B).
    - Task stack change.
- NoAction().

### 3.2 Operations on Task 
There are four operations on task as followed:

- LaunchAct(B): Push the instance of B into the top task.
    - eg. Launch an standard activity B.
- ClearTop(B): Pop activity until the top of the top task is the instance of B.
    - eg. Launch an singleTask activity B.
- ClearTask(B): Pop activity until the top task is empty, and Push(B).
    - eg. Launch an activity B with intent flag `FLAG_ACTIVITY_CLEAR_TASK`.
- MoveAct2Top(B): Move the first instance of B to the top of the top task.
    - eg. Launch an activity B with intent flag `FLAG_ACTIVITY_REORDER_TO_FRONT`.
- NoAction()

## 4 Understanding Intent Flags

In this section I will tell you how to decide the behavious of `A.startActivity(B, Fs)`, 
which means A startActivity B with the intent flags Fs.

We don't consider the intent flag `FLAG_ACTIVITY_REORDER_TO_FRONT` first, I will talk about this later. And suppose the launch mode of caller activity A is not `singleInstance`.

### 4.1 `Lmd(B) = singleInstance`
#### 4.1.1 Methodology
##### Find a task S via [SingleInstance Task Allocation Mechanism](#21).
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack, then **MoveTask2Top(S)**;
- if S is not existed, then **LaunchTask(B)**.

#### 4.1.2 Expriments
- S is the top task

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.1.png)

- S is not the top task

![MoveTask2Top(S)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleInstance.png)

- S is not founded

![LaunchTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)

### 4.2 `Lmd(B) = singleTask`
#### 4.2.1 Methodology
##### a. Find a task S via Non-SingleInstance Task Allocation Mechanism.
##### b. Operate on task stack
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack, then **MoveTask2Top(S)**;
- if S is not existed, then **LaunchTask(B)**.

##### c. Operate on task 
- if `FLAG_ACTIVITY_CLEAR_TASK` is in Fs, then **ClearTask(B)**;
- otherwise, 
    - if there exists an instance which class is B in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.

#### 4.2.2 Expriments
- S is the top task

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_CLEAR_TASK} | {} ||**ClearTask(B)**|
| {} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is in S|**ClearTop(B)**|
| {} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is not in S|**LaunchAct(B)**|

![ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.1.png)

![ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.2.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

- S is not the top task

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_CLEAR_TASK} | {} ||**MoveTask2Top(S) <br> ClearTask(B)**|
| {} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is in S|**MoveTask2Top(S) <br> ClearTop(B)**|
| {} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is not in S|**MoveTask2Top(S) <br> LaunchAct(B)**|

![MoveTask2Top(S) + ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.6.png)

![MoveTask2Top(S) + ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.9.png)

![MoveTask2Top(S) + LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.10.png)

- S is not existed

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {} | {} ||**LaunchTask(B)**|

![LaunchTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/singleTask1.png)

### 4.3 `Lmd(B) = standard`
#### 4.3.1 Methodology
##### Decide whether to allocate task.
- if ~~FLAG_ACTIVITY_NEW_TASK~~,  goto [Method 1](#4312).
- if FLAG_ACTIVITY_NEW_TASK and ~~FLAG_ACTIVITY_MULTIPLE_TASK~~, then goto [Method 2](#4311).
- if FLAG_ACTIVITY_NEW_TASK and FLAG_ACTIVITY_MULTIPLE_TASK, then **LaunchTask(B)**

<h5 id=4312>Method 1</h5>

- if FLAG_ACTIVITY_CLEAR_TOP,
    - if B is in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.
- if FLAG_ACTIVITY_REORDER_TO_FRONT, ~~FLAG_ACTIVITY_CLEAR_TOP~~,
    - if B is in the top task, then **MoveAct2Top(B)**;
    - otherwise, **LaunchAct(B)**.
- if FLAG_ACTIVITY_SINGLE_TOP, ~~FLAG_ACTIVITY_REORDER_TO_FRONT~~, ~~FLAG_ACTIVITY_CLEAR_TOP~~,
    - if B is the top of the top task, then **NoAction()**;
    - otherwise, **LaunchAct(B)**.
- if ~~FLAG_ACTIVITY_SINGLE_TOP~~, ~~FLAG_ACTIVITY_REORDER_TO_FRONT~~, ~~FLAG_ACTIVITY_CLEAR_TOP~~,
<br> then **LaunchAct(B)**.


<h5 id=4311>Method 2</h5>

###### a. Find a task S via Non-SingleInstance Task Allocation Mechanism.
###### b. Operate on task stack
- if S is the top task in the task stack, then **NoAction()**;
- if S is not the top task in the task stack, then **MoveTask2Top(S)**;
- if S is not existed, then **LaunchTask(B)**.

###### c. Operate on task 
- if FLAG_ACTIVITY_CLEAR_TASK, then **ClearTask(B)**;
- if FLAG_ACTIVITY_CLEAR_TOP, ~~FLAG_ACTIVITY_CLEAR_TASK~~,
    - if B is in the top task, then **ClearTop(B)**;
    - otherwise, **LaunchAct(B)**.
- if FLAG_ACTIVITY_SINGLE_TOP, ~~FLAG_ACTIVITY_CLEAR_TOP~~, ~~FLAG_ACTIVITY_CLEAR_TASK~~,
    - if B is the top of the top task, then **NoAction()**;
    - if B is the `realActivity` of S and B is not `mainActivity`, then **NoAction()**;
    - otherwise, **LaunchAct(B)**.
- if ~~FLAG_ACTIVITY_SINGLE_TOP~~, ~~FLAG_ACTIVITY_CLEAR_TOP~~, ~~FLAG_ACTIVITY_CLEAR_TASK~~,
    - if B is the `realActivity` of S and B is not `mainActivity`, then **NoAction()**;
    - otherwise, **LaunchAct(B)**.

#### 4.3.2 Expriments
##### Method 1

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_CLEAR_TOP} | {~~FLAG_ACTIVITY_NEW_TASK~~} |B is in the top task|**ClearTop(B)**|
| {FLAG_ACTIVITY_CLEAR_TOP} | {~~FLAG_ACTIVITY_NEW_TASK~~} |B is not in the top task|**LaunchAct(B)**|

![ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.2.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_REORDER_TO_FRONT} | {~~FLAG_ACTIVITY_NEW_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is in the top task|**MoveAct2Top(B)**|
| {FLAG_ACTIVITY_REORDER_TO_FRONT} | {~~FLAG_ACTIVITY_NEW_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is in the top task|**LaunchAct(B)**|

![MoveAct2Top(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.3.1.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_Single_TOP} | {~~FLAG_ACTIVITY_NEW_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~<br>~~FLAG_ACTIVITY_REORDER_TO_FRONT~~} |B is top |**NoAction()**|
| {FLAG_ACTIVITY_Single_TOP} | {~~FLAG_ACTIVITY_NEW_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~<br>~~FLAG_ACTIVITY_REORDER_TO_FRONT~~} |B is not top|**LaunchAct(B)**|

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.1.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

##### Method 2

- S is the top task

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_CLEAR_TASK} | {} ||**ClearTask(B)**|

![ClearTask(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.1.png)

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_CLEAR_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is in S|**ClearTop(B)**|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_CLEAR_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~} |B is not in S|**LaunchAct(B)**|

![ClearTop(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.2.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_SINGLE_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is top|**NoAction()**|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_SINGLE_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is not top;<br>B is not mainActivity;<br>B is realActivity of S|**NoAction()**|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_SINGLE_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is not top;<br>B is mainActivity;|**LaunchAct(B)**|
| {FLAG_ACTIVITY_NEW_TASK<br>FLAG_ACTIVITY_SINGLE_TOP} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~} |B is not top;<br>B is not realActivity of S;|**LaunchAct(B)**|

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.1.2.1.png)

![NoAction()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.3.2.1.png)

![LaunchAct()](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.3.2.2.png)

![LaunchAct(B)](https://github.com/LoringHe/TaskDroid/blob/master/pictures/4.2.2.3.png)

| Flags+ | Flags- | Task| Action |
| :----:   |  :----:  | :----:| :----:|
| {FLAG_ACTIVITY_NEW_TASK} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~<br>~~FLAG_ACTIVITY_SINGLE_TOP~~} |B is not mainActivity;<br>B is realActivity of S|**NoAction()**|
| {FLAG_ACTIVITY_NEW_TASK} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~<br>~~FLAG_ACTIVITY_SINGLE_TOP~~} |B is mainActivity;|**LaunchAct(B)**|
| {FLAG_ACTIVITY_NEW_TASK} | {~~FLAG_ACTIVITY_CLEAR_TASK~~<br>~~FLAG_ACTIVITY_CLEAR_TOP~~<br>~~FLAG_ACTIVITY_SINGLE_TOP~~} |B is not realActivity of S;|**LaunchAct(B)**|

