# teaming
Single IPC channel shared with multiples.

## Table on Contents
### Intro
### Prerequisite
### Installation
### Usage
### Concept

## Intro
This module developed to test efficient message exchanges between multiple processes, threads.
Concept and mechanisms are described below.

## Prerequisite
This module was developed on linux enviroment. Windows, OS X Hasn't tested yet.

## Installation
```
npm i teaming
```

## Usage
### Traditional Method

```javascript

const cluster = require('cluster');
if(cluster.isMaster){
    // Parent Process
    const emitter = new SocketEmitter();
    let semId = emitter.getSemId();

    let worker = cluster.fork({semId : semId});

    // On receive byte buffer from child
    emitter.on('data',buffer =>{
        //... Do something
    }).watch();

    // Send data to child
    emitter.send('data to send');
}else{
    // Child Process
    // Init SocketEmitter with semId from parent.
    const semId = parseInt(process.env['semId']);
    const emitter = new SocketEmitter(semId);
    // On receive byte buffer from master
    emitter.on('data',buffer =>{
        //... Do something
        // Send data back to parent
        emitter.send('data to send');
    }).watch();

}
```

### Manual Method
```javascript
const teamSocket = new TeamSocket();
let semId = teamSocket.getSemId();

let result = teamSocket.read();
if(result !== undefined){
    console.log(result);
}

teamSocket.write('data');

```

## Concept


