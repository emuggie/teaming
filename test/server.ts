import Express from 'express';
import Http from 'http';
import Cluster from 'cluster';

import { TimeoutPromise } from 'land-of-promise';

import { SocketEmitter } from '../lib';

if(Cluster.isMaster){
    const emitter = new SocketEmitter();
    
    let worker1 = Cluster.fork({semId : emitter.semId});
    let worker2 = Cluster.fork({semId : emitter.semId});

    const app = Express();
    app.set('port', process.env.PORT || 8090);

    // Pending Request map
    let pendingReqMap = new Map<string,TimeoutPromise<any>>();
    app.use('/',(req, res, next)=>{
        // Unique key for each request
        const messageId = new Date().valueOf().toString(36) + ':' + Math.random().toString(36).substr(2);
        // Parameter to hand over
        let jo = {
            id : messageId,
            uri : req.url
        }

        // Timeout promise
        let promise = new TimeoutPromise<any>((resolve, reject)=>{
            // send to child:any 
            if(!emitter.write(JSON.stringify(jo))){
                reject('buffer full');
            } 
        }).then(result =>{
            // response back
            res.send(`done : ${JSON.stringify(result)}`);
        }).catch(error=>{
            // on error
            res.send(`failed: ${error}`);
        });

        // register pending request
        pendingReqMap.set(messageId, promise);
    });

    //define bevior when received
    emitter.on('data', buffer=>{
        let returned = JSON.parse(buffer.toString());
        console.log('received', returned);
        // retrieve pending request from map with id
        let promise = pendingReqMap.get(returned.id);

        if(promise){
            // resovle if found
            promise.resolve(returned);
            // remove resolved.
            pendingReqMap.delete(returned.id);
        }
    }).watch();

    Http.createServer(app).listen(app.get('port'),()=>{
        console.log('server start');
    });
}else{
    // Child task

    //Init connection with parent
    let semId = -1;
    try{
        semId = parseInt(process.env['semId']||'');
    }catch(error){
        console.error(`Process ${process.pid} failed to acquire semaphore with id ${process.env['semId']}`);
        process.exit(1);
    }
    const emitter = new SocketEmitter(semId);

    // Define task given from parent.
    emitter.on('data',buffer =>{
        let jo = JSON.parse(buffer.toString());
        jo.response = 'received';
        //Write back to parent
        emitter.write(JSON.stringify(jo));
    }).watch();
}


