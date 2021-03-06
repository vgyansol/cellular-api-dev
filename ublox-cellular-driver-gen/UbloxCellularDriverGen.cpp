/* Copyright (c) 2017 ublox Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UbloxCellularDriverGen.h"
#include "string.h"
#if defined(FEATURE_COMMON_PAL)
#include "mbed_trace.h"
#define TRACE_GROUP "UCID"
#else
#define debug_if(_debug_trace_on, ...) (void(0)) // dummies if feature common pal is not added
#define tr_info(...)  (void(0)) // dummies if feature common pal is not added
#define tr_error(...) (void(0)) // dummies if feature common pal is not added
#endif

/**********************************************************************
 * PROTECTED METHODS: Short Message Service
 **********************************************************************/

// URC for Short Message listing.
void UbloxCellularDriverGen::CMGL_URC()
{
    char buf[64];
    int index;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +CMGL: <ix>,...
    *buf = 0;
    if (read_at_to_char(buf, sizeof(buf), '\n') > 0) {
        // Now also read out the text message, so that we don't
        // accidentally trigger URCs or the like on any of
        // its contents
        *_smsBuf = 0;
        read_at_to_char(_smsBuf, sizeof(_smsBuf), '\n');
        // Note: don't put any debug in here, this URC is being
        // called multiple times and debug may cause it to
        // miss characters
        if (sscanf(buf, ": %d,", &index) == 1) {
            _smsCount++;
            if ((_userSmsIndex != NULL) && (_userSmsNum > 0)) {
                *_userSmsIndex = index;
                _userSmsIndex++;
                _userSmsNum--;
            }
        }
    }
}

// URC for new SMS messages.
void UbloxCellularDriverGen::CMTI_URC()
{
    char buf[32];

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    *buf = 0;
    if (read_at_to_char(buf, sizeof (buf), '\n') > 0) {
        // No need to parse, any content is good
        tr_info("New SMS received");
    }
}

/**********************************************************************
 * PROTECTED METHODS: Unstructured Supplementary Service Data
 **********************************************************************/

// URC for call waiting.
void UbloxCellularDriverGen::CCWA_URC()
{
    char buf[32];
    int numChars;
    int a;
    int b = 0;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +CCWA: <status>[, <class>]
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        if (sscanf(buf, ": %d, %d", &a, &b) > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+CCWA", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    if (a > 0) {
                        debug_if(_debug_trace_on, "Calling Waiting is active");
                    } else {
                        debug_if(_debug_trace_on, "Calling Waiting is not active");
                    }
                    if (b > 0) {
                        if (b & 0x01) {
                            debug_if(_debug_trace_on, " for voice\n");
                        }
                        if (b & 0x02) {
                            debug_if(_debug_trace_on, " for data\n");
                        }
                        if (b & 0x04) {
                            debug_if(_debug_trace_on, " for fax\n");
                        }
                        if (b & 0x08) {
                            debug_if(_debug_trace_on, " for SMS\n");
                        }
                        if (b & 0x10) {
                            debug_if(_debug_trace_on, " for data circuit sync\n");
                        }
                        if (b & 0x20) {
                            debug_if(_debug_trace_on, " for data circuit async\n");
                        }
                        if (b & 0x40) {
                            debug_if(_debug_trace_on, " for dedicated packet access\n");
                        }
                        if (b & 0x80) {
                            debug_if(_debug_trace_on, " for dedicated PAD access\n");
                        }
                    }
                }
            }
        }
    }
}

// URC for call forwarding.
void UbloxCellularDriverGen::CCFC_URC()
{
    char buf[32];
    int numChars;
    char num[32];
    int a, b;
    int numValues;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +CCFC: <status>[, <class>]
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        memset (num, 0, sizeof (num));
        numValues = sscanf(buf, ": %d,%d,\"%32[^\"][\"]", &a, &b, num);
        if (numValues > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+CCFC", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    if (a > 0) {
                        debug_if(_debug_trace_on, "Calling Forwarding is active ");
                    } else {
                        debug_if(_debug_trace_on, "Calling Forwarding is not active ");
                    }
                    if (numValues > 1) {
                        if (b > 0) {
                            if (b & 0x01) {
                                debug_if(_debug_trace_on, " for voice");
                            }
                            if (b & 0x02) {
                                debug_if(_debug_trace_on, " for data");
                            }
                            if (b & 0x04) {
                                debug_if(_debug_trace_on, " for fax");
                            }
                            if (b & 0x08) {
                                debug_if(_debug_trace_on, " for SMS");
                            }
                            if (b & 0x10) {
                                debug_if(_debug_trace_on, " for data circuit sync");
                            }
                            if (b & 0x20) {
                                debug_if(_debug_trace_on, " for data circuit async");
                            }
                            if (b & 0x40) {
                                debug_if(_debug_trace_on, " for dedicated packet access");
                            }
                            if (b & 0x80) {
                                debug_if(_debug_trace_on, " for dedicated PAD access");
                            }
                        }
                    }
                    if (numValues > 2) {
                        debug_if(_debug_trace_on, " for %s\n", num);
                    } else {
                        debug_if(_debug_trace_on, "\n");
                    }
                }
            }
        }
    }
}


// URC for calling line ID restriction.
void UbloxCellularDriverGen::CLIR_URC()
{
    char buf[32];
    int numChars;
    int a, b;
    int numValues;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +CLIR: <n>[, <m>]
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        numValues = sscanf(buf, ": %d,%d", &a, &b);
        if (numValues > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+CLIR", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    switch (a) {
                        case 0:
                            debug_if(_debug_trace_on, "Calling Line ID restriction is as subscribed\n");
                            break;
                        case 1:
                            debug_if(_debug_trace_on, "Calling Line ID invocation ");
                            break;
                        case 2:
                            debug_if(_debug_trace_on, "Calling Line ID suppression ");
                            break;
                    }
                    if (numValues > 2) {
                        switch (b) {
                            case 0:
                                debug_if(_debug_trace_on, " is not provisioned\n");
                                break;
                            case 1:
                                debug_if(_debug_trace_on, " is provisioned permanently\n");
                                break;
                            case 2:
                                debug_if(_debug_trace_on, " is unknown\n");
                                break;
                            case 3:
                                debug_if(_debug_trace_on, " is in temporary mode, presentation restricted\n");
                                break;
                            case 4:
                                debug_if(_debug_trace_on, " is in temporary mode, presentation allowed\n");
                                break;
                        }
                    }
                }
            }
        }
    }
}

// URC for calling line ID presentation.
void UbloxCellularDriverGen::CLIP_URC()
{
    char buf[32];
    int numChars;
    int a, b;
    int numValues;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +CLIP: <n>[, <m>]
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        numValues = sscanf(buf, ": %d,%d", &a, &b);
        if (numValues > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+CLIP", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    switch (a) {
                        case 0:
                            debug_if(_debug_trace_on, "Calling Line ID disable ");
                            break;
                        case 1:
                            debug_if(_debug_trace_on, "Calling Line ID enable ");
                            break;
                    }
                    if (numValues > 1) {
                        switch (b) {
                            case 0:
                                debug_if(_debug_trace_on, " is not provisioned\n");
                                break;
                            case 1:
                                debug_if(_debug_trace_on, " is provisioned\n");
                                break;
                            case 2:
                                debug_if(_debug_trace_on, " is unknown\n");
                                break;
                        }
                    }
                }
            }
        }
    }
}

// URC for connected line ID presentation.
void UbloxCellularDriverGen::COLP_URC()
{
    char buf[32];
    int numChars;
    int a, b;
    int numValues;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +COLP: <n>[, <m>]
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        numValues = sscanf(buf, ": %d,%d", &a, &b);
        if (numValues > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+COLP", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    switch (a) {
                        case 0:
                            debug_if(_debug_trace_on, "Connected Line ID disable ");
                            break;
                        case 1:
                            debug_if(_debug_trace_on, "Connected Line ID enable ");
                            break;
                    }
                    if (numValues > 1) {
                        switch (b) {
                            case 0:
                                debug_if(_debug_trace_on, " is not provisioned\n");
                                break;
                            case 1:
                                debug_if(_debug_trace_on, " is provisioned\n");
                                break;
                            case 2:
                                debug_if(_debug_trace_on, " is unknown\n");
                                break;
                        }
                    }
                }
            }
        }
    }
}

// URC for connected line ID restriction.
void UbloxCellularDriverGen::COLR_URC()
{
    char buf[32];
    int numChars;
    int a;

    // Note: not calling _at->recv() from here as we're
    // already in an _at->recv()
    // +COLR: <status>
    numChars = read_at_to_char(buf, sizeof (buf), '\n');
    if (numChars > 0) {
        if (sscanf(buf, ": %d", &a) > 0) {
            if (_ssUrcBuf == NULL) {
                _ssUrcBuf = (char *) malloc(numChars + 5 + 1);
                if (_ssUrcBuf != NULL) {
                    memcpy (_ssUrcBuf, "+COLR", 5);
                    memcpy (_ssUrcBuf + 5, buf, numChars);
                    *(_ssUrcBuf + numChars + 5) = 0;
                    switch (a) {
                        case 0:
                            debug_if(_debug_trace_on, "Connected Line ID restriction is not provisioned\n");
                            break;
                        case 1:
                            debug_if(_debug_trace_on, "Connected Line ID restriction is provisioned\n");
                            break;
                        case 2:
                            debug_if(_debug_trace_on, "Connected Line ID restriction is unknown\n");
                            break;
                    }
                }
            }
        }
    }
}

/**********************************************************************
 * PUBLIC METHODS: Generic
 **********************************************************************/

// Constructor.
UbloxCellularDriverGen::UbloxCellularDriverGen(PinName tx, PinName rx,
                                               int baud, bool debug_on)
{
    _userSmsIndex = NULL;
    _userSmsNum = 0;
    _smsCount = 0;
    _ssUrcBuf = NULL;

    // Initialise the base class, which starts the AT parser
    baseClassInit(tx, rx, baud, debug_on);

    // URCs related to SMS
    _at->oob("+CMGL", callback(this, &UbloxCellularDriverGen::CMGL_URC));
    // Include the colon with this one as otherwise it could be found
    // by +CMT, should it ever occur
    _at->oob("+CMTI:", callback(this, &UbloxCellularDriverGen::CMTI_URC));

    // URCs relater to supplementary services
    _at->oob("+CCWA", callback(this, &UbloxCellularDriverGen::CCWA_URC));
    _at->oob("+CCFC", callback(this, &UbloxCellularDriverGen::CCFC_URC));
    _at->oob("+CLIR", callback(this, &UbloxCellularDriverGen::CLIR_URC));
    _at->oob("+CLIP", callback(this, &UbloxCellularDriverGen::CLIP_URC));
    _at->oob("+COLP", callback(this, &UbloxCellularDriverGen::COLP_URC));
    _at->oob("+COLR", callback(this, &UbloxCellularDriverGen::COLR_URC));
}

// Destructor.
UbloxCellularDriverGen::~UbloxCellularDriverGen()
{
}

/**********************************************************************
 * PUBLIC METHODS: Short Message Service
 **********************************************************************/

// Count the number of messages on the module.
int UbloxCellularDriverGen::smsList(const char* stat, int* index, int num)
{
    int numMessages = -1;
    LOCK();

    _userSmsIndex = index;
    _userSmsNum = num;
    _smsCount = 0;
    // There is a callback to capture the result
    // +CMGL: <ix>,...
    _at->debug_on(false); // No time for AT interface debug
                          // as the list comes out in one long
                          // stream and we can lose characters if we
                          // pause to do printfs
    if (_at->send("AT+CMGL=\"%s\"", stat) && _at->recv("OK\n")) {
        numMessages = _smsCount;
    }
    _at->debug_on(_debug_trace_on);

    // Set this back to null so that the URC won't trample
    _userSmsIndex = NULL;

    UNLOCK();
    return numMessages;
}

// Send an SMS message.
bool UbloxCellularDriverGen::smsSend(const char* num, const char* buf)
{
    bool success = false;
    char typeOfAddress = TYPE_OF_ADDRESS_NATIONAL;
    LOCK();

    if ((strlen (num) > 0) && (*(num) == '+')) {
        typeOfAddress = TYPE_OF_ADDRESS_INTERNATIONAL;
    }
    if (_at->send("AT+CMGS=\"%s\",%d", num, typeOfAddress) && _at->recv(">")) {
        if ((_at->write(buf, (int) strlen(buf)) >= (int) strlen(buf)) &&
            (_at->putc(0x1A) == 0) &&  // CTRL-Z
            _at->recv("OK")) {
            success = true;
        }
    }

    UNLOCK();
    return success;
}

bool UbloxCellularDriverGen::smsDelete(int index)
{
    bool success;
    LOCK();

    success = _at->send("AT+CMGD=%d", index) && _at->recv("OK");

    UNLOCK();
    return success;
}

bool UbloxCellularDriverGen::smsRead(int index, char* num, char* buf, int len)
{
    bool success = false;
    char * endOfString;
    int smsReadLength = 0;
    LOCK();

    if (len > 0) {
        //+CMGR: "REC READ", "+393488535999",,"07/04/05,18:02:28+08",145,4,0,0,"+393492000466",145,93
        // The text of the message.
        // OK
        memset (_smsBuf, 0, sizeof (SMS_BUFFER_SIZE)); // Ensure terminator
        if (_at->send("AT+CMGR=%d", index) &&
            _at->recv("+CMGR: \"%*[^\"]\",\"%15[^\"]\"%*[^\n]\n", num) &&
            _at->recv("%" stringify(SMS_BUFFER_SIZE) "[^\n]\n", _smsBuf) &&
            _at->recv("OK")) {
            endOfString = strchr(_smsBuf, 0);
            if (endOfString != NULL) {
                smsReadLength = endOfString - _smsBuf;
                if (smsReadLength + 1 > len) { // +1 for terminator
                    smsReadLength = len - 1;
                }
                memcpy(buf, _smsBuf, smsReadLength);
                *(buf + smsReadLength) = 0; // Add terminator
                success = true;
            }
        }
    }

    UNLOCK();
    return success;
}

/**********************************************************************
 * PUBLIC  METHODS: Unstructured Supplementary Service Data
 **********************************************************************/

// Perform a USSD command.
bool UbloxCellularDriverGen::ussdCommand(const char* cmd, char* buf, int len)
{
    bool success = false;
    char * tmpBuf;
    int atTimeout;
    int x;
    Timer timer;
    LOCK();
    atTimeout = _at_timeout; // Has to be inside LOCK()s

    if (len > 0) {
        *buf = 0;
        if (len > USSD_STRING_LENGTH + 1) {
            len = USSD_STRING_LENGTH + 1;
        }

        tmpBuf = (char *) malloc(USSD_STRING_LENGTH + 1);

        if (tmpBuf != NULL) {
            memset (tmpBuf, 0, USSD_STRING_LENGTH + 1);
            // +CUSD: \"%*d, \"%128[^\"]\",%*d"
            if (_at->send("AT+CUSD=1,\"%s\"", cmd)) {
                // Wait for either +CUSD to come back or
                // one of the other SS related URCs to trigger
                if (_ssUrcBuf != NULL) {
                    free (_ssUrcBuf);
                    _ssUrcBuf = NULL;
                }
                timer.start();
                _at->set_timeout(1000);
                while (!success && (timer.read_ms() < atTimeout)) {
                    if (_at->recv("+CUSD: %*d,\"")) {
                        // Note: don't wait for "OK" here as the +CUSD response may come
                        // before or after the OK
                        // Also, the return string may include newlines so can't just use
                        // recv() to capture it as recv() will stop capturing at a newline.
                        if (read_at_to_char(tmpBuf, USSD_STRING_LENGTH, '\"') > 0) {
                            success = true;
                            memcpy (buf, tmpBuf, len);
                            *(buf + len - 1) = 0;
                        }
                    } else {
                        // Some of the return values do not appear as +CUSD but
                        // instead as the relevant URC for call waiting, call forwarding,
                        // etc.  Test those here.
                        if (_ssUrcBuf != NULL) {
                            success = true;
                            x = strlen (_ssUrcBuf);
                            if (x > len - 1 ) {
                                x = len - 1;
                            }
                            memcpy (buf, _ssUrcBuf, x);
                            *(buf + x) = 0;
                            free (_ssUrcBuf);
                            _ssUrcBuf = NULL;
                        }
                    }
                }
                at_set_timeout(atTimeout);
                timer.stop();
            }
        }
    }

    UNLOCK();
    return success;
}

/**********************************************************************
 * PUBLIC: Module File System
 **********************************************************************/

// Delete a file from the module's file system.
bool UbloxCellularDriverGen::delFile(const char* filename)
{
    bool success;
    LOCK();

    success = _at->send("AT+UDELFILE=\"%s\"", filename) && _at->recv("OK");

    UNLOCK();
    return success;
}

// Write a buffer of data to a file in the module's file system.
int UbloxCellularDriverGen::writeFile(const char* filename, const char* buf, int len)
{
    int bytesWritten = -1;
    LOCK();

    if (_at->send("AT+UDWNFILE=\"%s\",%d", filename, len) && _at->recv(">")) {
        if ((_at->write(buf, len) >= len) && _at->recv("OK")) {
            bytesWritten = len;
        }
    }

    UNLOCK();
    return bytesWritten;
}

// Read a file from the module's file system
// Note: this is implemented with block reads since UARTSerial
// does not currently allow flow control and there is a danger
// of character loss with large whole-file reads
int UbloxCellularDriverGen::readFile(const char* filename, char* buf, int len)
{
    int countBytes = -1;  // Counter for file reading (default value)
    int bytesToRead = fileSize(filename);  // Retrieve the size of the file
    int offset = 0;
    int blockSize = FILE_BUFFER_SIZE;
    char respFilename[48 + 1];
    int sz, sz_read;
    bool success = true;
    int ch = 0;
    int timeLimit;
    Timer timer;

    debug_if(_debug_trace_on, "readFile: filename is %s; size is %d\n", filename, bytesToRead);

    memset(respFilename, 0, sizeof (respFilename));  // Ensure terminator
    if (bytesToRead > 0)
    {
        if (bytesToRead > len) {
            bytesToRead = len;
        }

        while (success && (bytesToRead > 0)) {

            if (bytesToRead < blockSize) {
                blockSize = bytesToRead;
            }
            LOCK();

            if (blockSize > 0) {
                if (_at->send("AT+URDBLOCK=\"%s\",%d,%d\r\n", filename, offset, blockSize) &&
                    _at->recv("+URDBLOCK: \"%48[^\"]\",%d,\"", respFilename, &sz) &&
                    (strcmp(filename, respFilename) == 0)) {

                    // Would use _at->read() here, but if it runs ahead of the
                    // serial stream it returns -1 instead of the number of characters
                    // read so far, which is not very helpful so instead use _at->getc() and
                    // a time limit. The time limit is twice the amount of time it should take to
                    // read the block at the working baud rate
                    timer.reset();
                    timer.start();
                    timeLimit = blockSize * 2 / ((MBED_CONF_UBLOX_CELL_BAUD_RATE / 8) / 1000);
                    sz_read = 0;
                    while ((sz_read < blockSize) && (timer.read_ms() < timeLimit)) {
                        ch = _at->getc();
                        if (ch >= 0) {
                            *buf = ch;
                            buf++;
                            sz_read++;
                        }
                    }
                    timer.stop();

                    if (sz_read == blockSize) {
                        bytesToRead -= sz_read;
                        offset += sz_read;
                        _at->recv("OK");
                    } else {
                        debug_if(_debug_trace_on, "blockSize %d but only received %d bytes\n", blockSize, sz_read);
                        success = false;
                    }
               } else {
                   success = false;
               }
            }

            UNLOCK();
        }

        if (success) {
            countBytes = offset;
        }
    }

    return countBytes;
}

// Return the size of a file.
int UbloxCellularDriverGen::fileSize(const char* filename)
{
    int returnValue = -1;
    int fileSize;
    LOCK();

    if (_at->send("AT+ULSTFILE=2,\"%s\"", filename) &&
        _at->recv("+ULSTFILE: %d\n", &fileSize) &&
        _at->recv("OK")) {
        returnValue = fileSize;
    }

    UNLOCK();
    return returnValue;
}

// End of file
