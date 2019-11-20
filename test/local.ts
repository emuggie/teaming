import { SocketEmitter } from '../lib';

import Cluster from 'cluster';

if(Cluster.isMaster){
    console.log('master init');
    let emitter = new SocketEmitter();
    console.log('master init1');
    
    console.log(process.pid,emitter.psfd,emitter.semId);
    for(let i =0;i<1;i++){
        let cp = Cluster.fork({ csfd : emitter.semId});
    }

    emitter.on('data', data=>{
        console.log('master received',data);
    }).watch();
    let i =0;
    setInterval(()=>{
        console.log('Master sends',Buffer.from('testing'), new Date(), i++);
        try{
            if(!emitter.write(Buffer.from('testing'))){
                throw 'write full';
            };
        }catch(e){
            console.log('master',e, i);
        }
    },1000);
}else{
    const csfd:number = parseInt(process.env['csfd']||'0');
    console.log(process.pid,csfd);
    
    const emitter =new SocketEmitter(csfd);
    emitter.on('data', data => {
        console.log(`child ${process.pid} received`, data, new Date());
        //emitter.write(Buffer.from('testing'));
    }).watch(1000);
}