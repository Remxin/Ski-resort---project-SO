# Ski-resort---project-SO


## Start virtual environment on Docker (optional)
1. Create docker project with development container

**This step needs to be done only once**
```
./setup_environment.sh
```
2. Start virtual environment

**When using Windows use this command in powershell**
```
docker exec -it c-dev-end /bin/bash 
```

3. Troubleshooting **(skip when no error)**

If there are some errors in starting virtual environment take those steps:

a) change folder ownership and permissions
```
sudo chown -R $USER {project folder}
chmod -R 755 {project folder}
```

b) Add current folder to file sharing on Docker
- settings -> Resouces -> File Sharing
- scroll down and enter an absolute path to your folder
- click plus icon to add the folder
- click 'Apply & restart' button
- if this stil does not work try moving folder to one of the listed paths at 'File sharing' settings. Then retake steps at subsection **a)**.


## Compile and start project
1. If created virtual environment - enter it

When you have compiled the program on your **local machine** and then you want to start it on the **virtual one**, you may expect **run error** due to system architecture differences. To avoid that first of all you need to process this command:
```
make clean
```

2. Compile the program
```
make
```

3. Run the program
```
make run
```