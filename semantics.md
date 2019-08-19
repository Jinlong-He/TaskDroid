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
- `singleTop`: 
    - The difference from `standard` is if there exists an instance of activity at the top of current task, it will be reused instead of instantiating a new one.
- `singleTask` 
    - There is only one instance of activity which set this launch mode in one task.
    - The Task affinity of the instance of activity is the task affinity of the task stored it.
- `singleInstance`
    - There is only one instance of activity which set this launch mode in the whole task stack.
    - No activities instance can be pushed into the same task of this instance's.

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

## 2 Semantics of `A.StartActivity(B, Fs)`
In the following, I will talk about how the contents of the task stack change, 
when calling function `A.StartActivity(B, Fs)`,
which means A startActivity B with the intent flags Fs.

For simply speaking, we define some notations.
- `Lmd(A)` return the launch mode of the activity A.
- `Aft(A)` return the task affinity of the activity A.
- `Rat(S)` return the realActivity of the task S.
- `Aft(S)` return the task affinity of the task S.

<h3 id=21>2.1 Case without Intent Flags</h3>
We first suppose `Lmd(A) != SIT`. I will talk about this in the [2.2](). 
#### 2.1.1 `Lmd(B) = STD`
B will be pushed into the top task directly.

![standard](https://github.com/LoringHe/Android-Multitasking-Mechanism/blob/master/pictures/standar.png)

#### 2.1.2 `Lmd(B) = STP`
If B is the different instance with A, B will be pushed into the top task.

#### 2.1.3 `Lmd(B) = STK`
- if there exists a task which task affinity is equal to `Aft(B)` and its realActivity's lauch mode is not `SIT`, then that task will be moved to the top of task stack first, and
    - if B is not in the top task, B will be pushed into the top task directly.
    - otherwise, the top task will pop until B is on the top.
- otherwise, it will create a new task which realActivity is B, and this task will be pushed into task stack.

#### 2.1.4 `Lmd(B) = SIT`
- if there is no one task which realActivity is B, it will create a new task which realActivity is B, and this task will be pushed into task stack.
- otherwise, the task which realAcitivity is B will be moved to the top of task stack.

### 2.2 General Case
#### 2.2.1 `Lmd(B) = SIT`
This case is special, because there is only one factor can change its behavious - the intent flag `FLAG_ACTIVITY_TASK_ON_HOME`.

Recall the behavious we mentioned before, 
if `FLAG_ACTIVITY_TASK_ON_HOME` is in `Fs`,

It is not hard to figure out how task stack generates from above cases, there are two important steps for generating.
- a. Search for task.
- b. Operates on task.

#### 2.1.1 How to Search for Task
Now I will talk about the first step first, when one of the following conditions is satisfied, [step a]() will be executed.

- 1) `Lmd(B) = SIT`
- 2) `Lmd(A) = SIT`
- 3) `Lmd(B) = STK`
- 4) `NTK in Fs`

We define a function `getTask(B, TS)` to repersent the process of [step a](), where `TS` means the current task stack.

    Task getTask(B, TS) {
        if (Lmd(B) = SIT) {
            for (Task S from TS.top() to TS.bot())
                if (Rat(S) = B) return S;
            return null;
        } else {
            for (Task S from TS.top() to TS.bot())
                if (Rat(S) = B) return S;
            for (Task S from TS.top() to TS.bot())
                if (Aft(S) = Aft(B)) return S;
            return null;
        }
    } 

Simply speaking, if [`Lmd(B) = SIT`](), there exists a task S in the task stack and `Rat(S) = B`, return S.

- else if there exists a task S in the task stack and `Rat(S) = B`, return the first S.
- else if there exists a task S which `Aft(S) = Aft(B)`, return the first S.
- else, return null.

