/// <reference types="node" />
import Event from 'events';
declare class SocketEmitter extends Event.EventEmitter {
    private socket;
    private watching;
    private interval;
    nextExec?: number;
    constructor(semId?: number);
    watch(interval?: number): void;
    unwatch(): void;
    write(data: string | Buffer): boolean;
    readonly psfd: number;
    readonly csfd: number;
    readonly semId: number;
    private readOnEventLoop;
    on(event: "data", listener: ((buffer: Buffer) => void)): this;
}
export { SocketEmitter };
