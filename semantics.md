# Semantics of Android Mutitasking Mechanism
In the following, I will introduce some basic attributes of Activity in Android System.
## 1 Basic Concepts
### 1.1 Launch Mode
There are four launch modes in Android System :
"Standard", "Single Top", "Single Task", "Single Instance".
We can set `android:launchMode` as one of the above launch modes of in the AndroidManifest.xml.
#### Standard 
The default value of Activity.
#### Single Top

#### Single Task
#### Single Instance
### 1.2 Task Affinity
The task affinity of an activity indicates to which task the activity prefers to belong.
### 1.3 Intent Flag
– FLAG_ACTIVITY_NEW_TASK (NTK)
– FLAG_ACTIVITY_CLEAR_TOP (CTP)
– FLAG_ACTIVITY_SINGLE_TOP (STP)
– FLAG_ACTIVITY_CLEAR_TASK (CTK)
– FLAG_ACTIVITY_MULTIPLE_TASK (MTK)
– FLAG_ACTIVITY_TASK_ON_HOME (TOH)
– FLAG_ACTIVITY_REORDER_TO_FRONT (RTF)

