#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ku_pir.h"

int main(int argc, char* argv[]){
	int fd;
	int ret;
	
	fd = ku_pir_open();
	printf("fd %d\n", fd);
	
	struct ku_pir_data *data;
	data = (struct ku_pir_data*)malloc(sizeof(struct ku_pir_data));

	ku_pir_flush(fd);
	ku_pir_insertData(fd, 51, '1');
	ku_pir_insertData(fd, 52, '0');
	ku_pir_insertData(fd, 53, '1');
	ku_pir_flush(fd);
	printf("flush test, ts:%ld fg:%c\n", data->timestamp, data->rf_flag);

	ku_pir_insertData(fd, 40, '1');
	ku_pir_insertData(fd, 41, '0');
	ku_pir_insertData(fd, 42, '1');
	for(;;){
	ku_pir_read(fd, data);
	printf("flush test, ts:%ld fg:%c\n", data->timestamp, data->rf_flag);
	}

}
