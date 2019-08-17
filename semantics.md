# Semantics of Android Mutitasking Mechanism
## 1 Basic Concepts
<h3 id=11>1.1 Launch Mode</h3>
- standard (STD)
- singleTop (STP)
- singleTask (STK)
- singleInstance (SIT)

<h3 id=12>1.2 Task Affinity</h3>
The task affinity of an activity indicates to which task the activity prefers to belong.

``` xml 
    <activity android:launchMode=["standard" | "singleTop" | "singleTask" | "singleInstance"];
              android:taskAffinity="string">
    </activity>
```
We can both set [lauch mode](#11) and [task affinity](#12) of an Activity in the [Manifest.xml](https://developer.android.com/guide/topics/manifest/manifest-intro).
### 1.3 Intent Flag
- FLAG_ACTIVITY_NEW_TASK (NTK)
- FLAG_ACTIVITY_CLEAR_TOP (CTP)
- FLAG_ACTIVITY_SINGLE_TOP (STP)
- FLAG_ACTIVITY_CLEAR_TASK (CTK)
- FLAG_ACTIVITY_MULTIPLE_TASK (MTK)
- FLAG_ACTIVITY_TASK_ON_HOME (TOH)
- FLAG_ACTIVITY_REORDER_TO_FRONT (RTF)

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

### 1.4 Back Stack(Task)
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
- `affinity` is the task affinity of `realActivity`.

### 1.5 Task Stack
A task stack is used to store the back stacks for different tasks.
<h2 id=2>2 Semantics of `A.StartActivity(B, Fs)`</h2>
In the following, I will talk about how the contents of the task stack change, 
when calling function `A.StartActivity(B, Fs)`,
which means A startActivity B with the intent flags Fs.

For simply speaking, we define some notations.
- `Lmd(A)` return the launch mode of A.
- `Aft(A)` return the task affinity of A.

### 2.1 Case without Intent Flags.
We first suppose `Lmd(A) != SIT`. 
#### 2.1.1 `Lmd(B) = STD`
B will be pushed into task directly.

