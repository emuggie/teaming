import Event from 'events';

import { TeamSocket } from './team-socket';

class SocketEmitter extends Event.EventEmitter{
    private socket:TeamSocket;
    private watching = false;
    private interval:number = 0 ;
    nextExec?:number;

    constructor(semId?:number){
        super();
        if(semId){
            this.socket = new TeamSocket(semId);
        }else{
            this.socket = new TeamSocket();
        }
    }

    watch(interval?:number):void{
        if(interval && interval >= 0){
            this.interval = interval;
        }
        if(!this.watching){
            this.watching = true;
            this.readOnEventLoop(this);
        }
    }

    unwatch():void{
        this.watching = false;
        if(this.nextExec){
            clearTimeout(this.nextExec);
            this.nextExec = undefined;
        }
    }

    write(data:string|Buffer){
        return this.socket.write(data);
    }

    get psfd():number{
        return this.socket.getSfd();
    }

    get csfd():number{
        return this.socket.getCsfd();
    }
    
    get semId():number{
        return this.socket.getSemId();
    }

    private readOnEventLoop(context:this):void{
        try{
            let val = context.socket.read();
            // console.log('read',process.pid,val);
            if(val){
                context.emit("data", val);
            }
        }catch(e){
            console.warn('SocketEmitter',e);
        }
        if(context.watching){
            context.nextExec = setTimeout(context.readOnEventLoop, context.interval, context);
        }
    }
    on(event: "data", listener:((buffer:Buffer) => void)): this{
        return super.on(event, listener);
    }
}

export { SocketEmitter };