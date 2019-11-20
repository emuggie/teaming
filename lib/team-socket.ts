import bindings from 'bindings';

export interface TeamSocket {
    getSfd():number;
     getCsfd():number;
     getSemId():number;
     read():string;
     write(data:string|Buffer):boolean;    
}

export var TeamSocket : {
    new(sfd?:number) : TeamSocket,
} = bindings('teaming').TeamSocket;

