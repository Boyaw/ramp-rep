### TODO
- bug when running two threads


### Steps
1. define when the key is not found, what should we do
2. timestamp generate correctly? yes
3. map key to partition - done




### Error Message
we are in run_client
we are in while loop for client 1
this time we write 1
write succeed 1

we are in while loop for client 1
this time we write 1
write succeed 1

we are in while loop for client 1
this time we write 1
write succeed 1

we are in while loop for client 1
this time we read 1
get_all_items succeed 1

we are in while loop for client 1
this time we read 1
get_all_items succeed 1

we are in run_client
we are in while loop for client 2
this time we read 2
get_all_items succeed 2

we are in while loop for client 2
this time we write 2
write succeed 2

we are in while loop for client 2
this time we read 2
get_all_items succeed 2

we are in while loop for client 2
this time we write 2
write succeed 2

we are in while loop for client 2
this time we write 2
write succeed 2