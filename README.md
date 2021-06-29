# Welcome to TaskDroid!
---
TaskDroid is an Android App static analysis tool based on Android multi-tasking mechanism.

The semantics of Android multi-tasking mechanism is in [Understanding Android Mutitasking Mechanism](https://github.com/Jinlong-He/TaskDroid/blob/master/semantics.md).
---
# How to build
```
python3 build.py
```
# Command of TaskDroid
--input-file=filename  Use `filename` as input apk file to build model.

--output-file=filename  Use `filename` as output file to record output infomation.

--verify='boundedness'|'backHijacking'  Select property to verify.

--engine='nuxmv'|'decidable'  Select engine to verify.

--k=num Check property in `num`-configuration with `--engine=nuxmv`.
# Example
```
python3 run.py --input-file=example.apk --output-file=out.txt --verify=boundedness --engine=nuxmv --k=10
```

