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

<h2 id=2>2 Semantics of ``StartActivity()``</h2>

### 2.1 Case Standard
