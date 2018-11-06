#pragma once

/**
 * declare global network config variables in this file
 * 
 * define const variables in a head file is valid
 * 
 */


const int SERVER_PORT = 7500;
const int SERVER_REQ_QUEUE_LENGTH = 4; // server request queue max length, usually 2-4
const int BUF_LENGTH = 1024;					 // server buffer length MUST equals client buffer length