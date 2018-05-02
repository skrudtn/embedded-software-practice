#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ku_pir.h"

int main(){
	int fd;
	int ku_insert;
	int i;

	struct ku_pir_data *data;
	
	data = (struct ku_pir_data *)malloc(sizeof(struct ku_pir_data));
	i = 0;

	fd = ku_pir_open();
	
	printf("fd %d\n", fd);
	
	for(i=0;i<10;i++){
		ku_insert = ku_pir_insertData(fd, ++i, '1');
		sleep(1);
		ku_insert = ku_pir_insertData(fd, ++i, '0');
		sleep(1);
	}
}
