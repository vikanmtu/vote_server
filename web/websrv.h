//process http request
//returns 0 for error or length of answer
short websrv_put(unsigned char* buf, short max, short len);

//get data after tcp_request will be answered 
//returns length of data or 0 if no data
short websrv_get(unsigned char* buf, int max);

//return final answer
short websrv_end(unsigned char* buf, int max);

//return busy webpage
short websrv_busy(unsigned char* buf, short max);

//return deny page
short websrv_deny(unsigned char* buf, short max);



