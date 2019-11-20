if(process.argv.indexOf('--local')>-1){
    require('./local');
}else if(process.argv.indexOf('--server')>-1){
    require('./server');
}else{
    require('./local');
}
