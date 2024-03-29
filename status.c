#include <string.h>
#include "status.h"

void messageCode(StatusCode code, char *msg) {
	switch (code) {
        //success message
        case LOGIN_SUCCESS: strcpy(msg, "Login Successfully!!");
        break;
        case REGISTER_SUCCESS: strcpy(msg, "Register Successfully!!");
        break;
        case LOGOUT_SUCCESS: strcpy(msg, "Logout SuccessFully!!");
        break;
        //error message
        case USER_NOT_FOUND: strcpy(msg, "User Not Found!!");
        break;
        case USER_IS_BLOCKED: strcpy(msg, "User is Blocked!!");
        break;
        case BLOCKED_USER: strcpy(msg, "Blocked user!!");
        break;
        case PASSWORD_INVALID: strcpy(msg, "Your input password invalid!!");
        break;
        case USER_IS_ONLINE: strcpy(msg, "The User is Online!!");
        break;
        case ACCOUNT_IS_EXIST: strcpy(msg, "Account existed!!");
        break;
        //invalid message
        case FILE_NOT_FOUND: strcpy(msg, "File Not Found!!");
        break;
        case COMMAND_INVALID: strcpy(msg, "Command was invalid!!");
        break;
        case USERNAME_OR_PASSWORD_INVALID: strcpy(msg, "Username or Password invalid!!");
        break;
        // server error message
        case SERVER_ERROR: strcpy(msg, "Something error!!");
        break;
        default: break;
      }
}