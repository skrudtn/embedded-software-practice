#include <stdio.h>
#include <stdlib.h>
#include "ku_pir.h"

int main(){
	int fd;
	int ku_insert;

	struct ku_pir_data *data;
	
	data = (struct ku_pir_data *)malloc(sizeof(struct ku_pir_data));

	fd = ku_pir_open();

	for(;;){
	ku_pir_read(fd, data);
	printf("ts : %ld, flag:",data->timestamp);
	if(data->rf_flag == '1')
		printf("FALLING!!\n");
	else
		printf("RISING!!\n");
	}
}
