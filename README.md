# Welcome to TaskDroid!
---
TaskDroid is an Android App static analysis tool based on Android multi-tasking mechanism.
---
# How to build
```
python3 build.py
```
# Commond of TaskDroid
--input-file=filename  Use `filename` as input apk file to build model.

--output-file=filename  Use `filename` as output file to record output infomation.

--verify='boundedness'|'backHijacking'  Select property to verify.

--engine='nuxmv'|'decidable'  Select engine to verify.

--k=num Check property in `k`-configuration with `--engine=nuxmv`.
# Example
```
python3 run.py --input-file=example.apk --output-file=out.txt --verify=boundedness --engine=nuxmv --k=10
```

