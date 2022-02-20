# how to run server
```console
foo@bar:~$ make && build/apps/server <PORT>
```

# how to run client
```console
foo@bar:~$ make && build/apps/client <remoteIP> <remotePort>
```

|    URI       | HTTP Method |   POST body   |     Result    			   		  |
| ------------ | ----------- | ------------- | ---------------------------------- | 
| authenticate |    POST     | JSON string   | login     				   		  |
| register     |    POST     | JSON string   | registers new user		   	      |
| unregister   |    POST     | JSON string   | unregisters user  			      |
| list         |    GET      | empty         | lists users              		  |
| reset        |    POST     | JSON string   | resets password			  	      |
| get          |    GET      | JSON string   | get user information from user ID  |




# API Status Codes

Example of a response with status code and etc in JSON format.
```json
{
  "success": false,
  "status_code": 5,
  "status_message": "Login fail, wrong password"
}
```

|    Code    | HTTP Status |   Message  															    |
| ---------- | ----------- | -------------------------------------------------------------------------- |
|    1       | 200         |  Success  															 	    |  	
|    2       | 201         |  The request has been fulfilled, resulting in the creation of new resource |
|    3       | 400         |  Could not interpret the request      										|
|    4       | 401         |  Incorrect username 			 								 			|
|    5       | 401         |  Incorrect password      		 								 			|
|    6       | 403         |  Request denied, client not logged in     		 		  					|
|    7       | 409         |  Registration declined, username already exists   		  				    |
|    8       | 404         |  User not found								   		  					|
|    9       | 403         |  Wrong password    													    |
|    10      | 401         |  Access token needed for operation 										|	
|    11      | 401         |  Invalid access token														|
|    12      | 404         |  Cannot remove a user that doesn't exist									|
|    13      | 400         |  The request is missing a required parameter								|
|    14      | 404         |  User not found															|



Commands for client application

|    Command     |    Why ?         		   |
| -------------- | --------------------------- |
|    search      | search for something		   |
|    reset       | 	resets password			   |
|    exit        |  exits application		   |
|    help        |  if stuck				   |
|    list        |  prints users			   |
|    unregister  |  unregisters user		   |
