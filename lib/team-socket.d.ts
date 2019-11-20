/// <reference types="node" />
export interface TeamSocket {
    getSfd(): number;
    getCsfd(): number;
    getSemId(): number;
    read(): string;
    write(data: string | Buffer): boolean;
}
export declare var TeamSocket: {
    new (sfd?: number): TeamSocket;
};
