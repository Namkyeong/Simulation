/* External definitions for gohat. */
#include <stdio.h>
#include "simlib.h"             /* Required for use of simlib.c. */

#define EVENT_ARRIVAL1       1  /* Event type for arrival of group1 customer. */
#define EVENT_ARRIVAL2       2  /* Event type for arrival of group2 customer. */
#define EVENT_ARRIVAL3       3  /* Event type for arrival of group3 customer. */
#define EVENT_DEPARTURE      4  /* Event type for departure of a customer. */
#define EVENT_CLOSE_DOORS    5  /* Event type for closing doors at 2 P.M. */
#define START_STATISTICS     6  /* Event type for starting statistic */
#define LIST_QUEUE1          1  /* List number for group1,2 queue. */
#define LIST_QUEUE2          2  /* List number for group3 queue. */
#define SAMPST_DELAYS        1  /* sampst variable for delays in queue(s). */
#define SAMPST_SERVICES      2	/* sampst variable for service times */

/* Declare non-simlib global variables. */

int   num_tables1, num_tables2, serviced_customers, customer_type, num_balking,
	  RN_interarrival1,RN_interarrival2,RN_interarrival3,RN_service,RN_balking, serviced1, serviced2, serviced3;
float  length_doors_open, departure_time, warm_up;
FILE  *outfile;

/* Declare non-simlib functions. */

void arrive1(int RN_interarrival1, int RN_service, int RN_balking);
void arrive2(int RN_interarrival2, int RN_service, int RN_balking);
void arrive3(int RN_interarrival3, int RN_service, int RN_balking);
void depart(int list_table, int customer_type, int RN_service);
void report(void);
void balking(int customer_type, int RN_balking);
void init_model(void);

main()  /* Main function. */
{	
	float start_statistic;
	
	/* parameters */
	length_doors_open = 2.5;
	start_statistic = 0.5;
	
	outfile = fopen("gohat_final_s.out", "w");
	fprintf(outfile, "Gohat closes after%16.3f hours\n", length_doors_open);
    fprintf(outfile,"\n	      mean	mean	average");
	fprintf(outfile,"\n policy	      waiting	service	customer");
	fprintf(outfile,"\n(1~2,3~4)      time	      	time	in queue     served		balking 	");
	fprintf(outfile, "\n____________________________________________________________________");
	
	RN_interarrival1 = 4;
	RN_interarrival2 = 22;
	RN_interarrival3 = 9;
	RN_service = 6;
	RN_balking = 2;
	
	/* Open  and Write out files, report heading and input parameters.*/
	fprintf(outfile, "\n stream %2d %2d %2d %2d %2d\n", RN_interarrival1, RN_interarrival2, RN_interarrival3, RN_service, RN_balking);
						
	num_tables1 = 10;
					    
	for (num_tables2 = 1; num_tables2 <= 6 ; ++num_tables2){
					    	
	   	init_simlib();
	   	init_model();
					
	    /* Set maxatr = max(maximum number of attributes per record, 4) */
	    maxatr = 5;
						
	    /* Schedule the first arrival. */
	    event_schedule(generate1(RN_interarrival1), EVENT_ARRIVAL1);
	    event_schedule(generate2(RN_interarrival2), EVENT_ARRIVAL2);
		event_schedule(generate3(RN_interarrival3), EVENT_ARRIVAL3);
			
		/* Schedule the gohat closing, and start Statistic*/
	    event_schedule(60 * length_doors_open, EVENT_CLOSE_DOORS);
	    event_schedule(60 * start_statistic, START_STATISTICS);
						
	    /* Run the simulation while the event list is not empty. */
	    while (list_size[LIST_EVENT] != 0) {
						
	        /* Determine the next event. */
	        timing();
						
	        /* Invoke the appropriate event function. */
	        switch (next_event_type) {
	            case EVENT_ARRIVAL1:
	                arrive1((int) RN_interarrival1, (int) RN_service, (int) RN_balking);
	                break;
	            case EVENT_ARRIVAL2:
	                arrive2((int) RN_interarrival2, (int) RN_service, (int) RN_balking);
	                break;
	            case EVENT_ARRIVAL3:
	                arrive3((int) RN_interarrival3, (int) RN_service, (int) RN_balking);
	                break;     
	            case EVENT_DEPARTURE:
	                depart((int) transfer[3], (int) transfer[4], (int) RN_service);  /* transfer[3] is table_list. */
	                break;
	            case EVENT_CLOSE_DOORS:
	                event_cancel(EVENT_ARRIVAL1);
	                event_cancel(EVENT_ARRIVAL2);
	                event_cancel(EVENT_ARRIVAL3);
	                break;
				case START_STATISTICS:
					init_model();
					sampst(0.0, 0);
					timest(0.0, 0);			            
		    }
		}
		/* Report results for the simulation with num_tellers tellers. */
		report();	
		num_tables1 -= 2;
	}
	fclose(outfile);
	return ;
}

void init_model(){
	
	/* Set serviced_Customers = 0 */
	serviced_customers = 0;
	num_balking = 0;
	serviced1 = 0;
	serviced2 = 0;
	serviced3 = 0;
} 

void arrive1(int RN_interarrival1, int RN_service, int RN_balking)  /* Event function for arrival of a customer to the bank. */
{
    int table;

    /* Schedule next arrival. */
    event_schedule(sim_time + generate1(RN_interarrival1), EVENT_ARRIVAL1);

    /* If a table is idle, start service on the arriving customer. */
    for (table = 1; table <= num_tables1 + num_tables2; ++table) {
        if (list_size[2 + table] == 0) {

            /* This table is idle, so customer has delay of zero. */
            sampst(0.0, SAMPST_DELAYS);

            /* Make this table busy (attributes are irrelevant). */
            list_file(FIRST, 2 + table);

            /* Schedule a service completion. */
            transfer[3] = 2 + table;  /* Define third attribute of type-two event-list record before event_schedule. */
            transfer[4] = 1;
			departure_time = sim_time + generate_service(RN_service);
			sampst(departure_time - sim_time, SAMPST_SERVICES);
            event_schedule(departure_time, EVENT_DEPARTURE);

            /* Return control to the main function. */
            return;
        }
    }

    balking(1, RN_balking);
}

void arrive2(int RN_interarrival2, int RN_service, int RN_balking)  /* Event function for arrival of a customer to the bank. */
{
    int table;

    /* Schedule next arrival. */
    event_schedule(sim_time + generate2(RN_interarrival2), EVENT_ARRIVAL2);

    /* If a table is idle, start service on the arriving customer. */
    for (table = 1; table <= num_tables1 + num_tables2; ++table) {
        if (list_size[2 + table] == 0) {

            /* This table is idle, so customer has delay of zero. */
            sampst(0.0, SAMPST_DELAYS);

            /* Make this table busy (attributes are irrelevant). */
            list_file(FIRST, 2 + table);

            /* Schedule a service completion. */
            transfer[3] = 2 + table;  /* Define third attribute of type-two event-list record before event_schedule. */
            transfer[4] = 2;
			departure_time = sim_time + generate_service(RN_service);
			sampst(departure_time - sim_time, SAMPST_SERVICES);
            event_schedule(departure_time, EVENT_DEPARTURE);

            /* Return control to the main function. */
            return;
        }
    }

    balking(2,RN_balking);
}

void arrive3(int RN_interarrival3, int RN_service, int RN_balking)  /* Event function for arrival of a customer to the bank. */
{
    int table;

    /* Schedule next arrival. */
    event_schedule(sim_time + generate3(RN_interarrival3), EVENT_ARRIVAL3);

    /* If a teller is idle, start service on the arriving customer. */
	for (table = 1; table <= num_tables2; ++table) {
	    if (list_size[2 + num_tables1 + table] == 0) {
	
	            /* This table is idle, so customer has delay of zero. */
	        sampst(0.0, SAMPST_DELAYS);
	
	            /* Make this table busy (attributes are irrelevant). */
	        list_file(FIRST, 2 + num_tables1 + table);
	
	            /* Schedule a service completion. */
	        transfer[3] = 2 + num_tables1 + table;  /* Define third attribute of type-two event-list record before event_schedule. */
	        transfer[4] = 3;
			departure_time = sim_time + generate_service(RN_service);
			sampst(departure_time - sim_time, SAMPST_SERVICES);
	        event_schedule(departure_time, EVENT_DEPARTURE);
	
	            /* Return control to the main function. */
	        return;
	    }
    }
    
    /* Place the customer at the end of the 3~4인 queue. */
    balking(3,RN_balking);
}


void depart(int list_table, int customer_type, int RN_service)  /* Departure event function. */
{	
	float time1, time2;
	
	if (customer_type == 1){
		serviced1 += 1;
	}
	if (customer_type == 2){
		serviced2 += 1;
	}
	if (customer_type == 3){
		serviced3 += 1;
	}

	serviced_customers += 1;
	
	if (list_table > 2 + num_tables1) {
		/* Check to see whether the queue is empty. */
		if (list_size[2] == 0){	
			if (list_size[1] == 0){
		/* all queues are empty, so make the table idle. */
        		list_remove(FIRST, list_table);
			}
			else {
		        /* The queue is not empty, so start service on a customer. */
		        list_remove(FIRST, 1);
		        sampst(sim_time - transfer[1], SAMPST_DELAYS);
		        transfer[3] = list_table;  /* Define before event_schedule. */
		        transfer[4] = customer_type;
		        departure_time = sim_time + generate_service(RN_service);
				sampst(departure_time - sim_time, SAMPST_SERVICES);
		        event_schedule(departure_time, EVENT_DEPARTURE);
	    	}
		}
	    else {
	    	if(list_size[1] == 0) { /*queue1은 비어있고, queue2만 있음*/ 
	    		/* The queue2 is not empty, so start service on a customer. */
		        list_remove(FIRST, 2);
		        sampst(sim_time - transfer[1], SAMPST_DELAYS);
		        transfer[3] = list_table;  /* Define before event_schedule. */
		        transfer[4] = customer_type;
		        departure_time = sim_time + generate_service(RN_service);
				sampst(departure_time - sim_time, SAMPST_SERVICES);
		        event_schedule(departure_time, EVENT_DEPARTURE);
			}
			else{ /*queue1도 있고, queue2도 있는 경우 => simlib에 새로운 함수 list_check 추가*/
			
				list_check(FIRST, 1);
				time1 = transfer[1];
				list_check(FIRST, 2);
				time2 = transfer[1];
				
				if(time1 < time2){
					list_remove(FIRST, 1);
			        sampst(sim_time - transfer[1], SAMPST_DELAYS);
			        transfer[3] = list_table;  /* Define before event_schedule. */
			        transfer[4] = customer_type;
			        departure_time = sim_time + generate_service(RN_service);
					sampst(departure_time - sim_time, SAMPST_SERVICES);
			        event_schedule(departure_time, EVENT_DEPARTURE);
				}
				else{
					list_remove(FIRST, 2);
			        sampst(sim_time - transfer[1], SAMPST_DELAYS);
			        transfer[3] = list_table;  /* Define before event_schedule. */
			        transfer[4] = customer_type;
			        departure_time = sim_time + generate_service(RN_service);
					sampst(departure_time - sim_time, SAMPST_SERVICES);
			        event_schedule(departure_time, EVENT_DEPARTURE);
				}
			} 
	    }
	}
	else {
		/* Check to see whether the queue is empty. */	
		if (list_size[1] == 0){
		/* The queue is empty, so make the table idle. */
        	list_remove(FIRST, list_table);
		}
	    else {
	        /* The queue is not empty, so start service on a customer. */
	        list_remove(FIRST, 1);
	        sampst(sim_time - transfer[1], SAMPST_DELAYS);
	        transfer[3] = list_table;  /* Define before event_schedule. */
	        transfer[4] = customer_type;
	        departure_time = sim_time + generate_service(RN_service);
			sampst(departure_time - sim_time, SAMPST_SERVICES);
	        event_schedule(departure_time, EVENT_DEPARTURE);
	    }
	}
	
}

void balking(int customer_type, int RN_balking)  /* Event function for arrival of a customer to the bank. */
{
	if (list_size[1] + list_size[2] >= 9){
		num_balking += 1;
	}
	else {
		if(list_size[1] + list_size[2] >= 6 ){
			if(lcgrand(RN_balking) <= 0.7){
				num_balking += 1;
			}
			else{
				transfer[1] = sim_time;
				if (customer_type == 3){
					list_file(LAST, 2);
				}
				else {
					list_file(LAST, 1);
				}
			}
		}
		else{
			transfer[1] = sim_time;
			if (customer_type == 3){
				list_file(LAST, 2);
			}
			else {
				list_file(LAST, 1);
			}		
		}
	}
	return ;
}


void report(void)  /* Report generator function. */
{
	float mean_queue1, mean_queue2;
	int max_queue1, min_queue1, max_queue2, min_queue2;
    /* Compute and write out estimates of desired measures of performance. */
    filest(1);
    mean_queue1 = transfer[1];
    filest(2);
    mean_queue2 = transfer[1];
    fprintf(outfile, "(%3d,%3d)%15.2f%15.2f%15.2f%15.2d%15.2d%15.2d%15.2d%15.2d\n", 
	num_tables1, num_tables2, sampst(SAMPST_DELAYS,-SAMPST_DELAYS), sampst(SAMPST_SERVICES,-SAMPST_SERVICES), 
	mean_queue1 + mean_queue2, serviced_customers, num_balking,
	serviced1, serviced2, serviced3);
}
