//
//  OpenVPNConnector.m
//  OVPNClient

#import <Foundation/Foundation.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

#import "OpenVPNConnector.h"

#include <sys/socket.h>
#include <arpa/inet.h>

@implementation OpenVPNConnector : NSObject

const int DEFAULT_PORT = 9544;
enum CONNECTION_STATUS currentState_ = STATUS_DISCONNECTED;

NSTask *task_;
NSFileHandle *file_;
NSPipe *pipe_;
NSThread *thread_;
bool bStopThread_ = false;
xpc_connection_t xpc_connection_ = nil;
bool bSockConnected_ = false;

int sock_ = -1;

-(id)init
{
    if (self = [super init])
    {
        eventDelegate_ = nil;
        self.port = DEFAULT_PORT;
    }
    return self;
}

- (void) setEventDelegate: (id <OpenVPNConnectorEvents>)delegate
{
    eventDelegate_ = delegate;
}

- (BOOL) installHelper: (NSString *)label
{
    BOOL result = NO;
    
    // check if helper already installed
    /*CFDictionaryRef existingJob = SMJobCopyDictionary(kSMDomainSystemLaunchd, (__bridge CFStringRef)label);
    if (existingJob) {
        result = YES;
        CFRelease(existingJob);
        return result;
    }*/

    NSDictionary* installedHelperJobData;

    installedHelperJobData  = (__bridge NSDictionary*)SMJobCopyDictionary( kSMDomainSystemLaunchd, (__bridge CFStringRef)label );
    if (installedHelperJobData)
    {
        NSString*       installedPath           = [[installedHelperJobData objectForKey:@"ProgramArguments"] objectAtIndex:0];
        NSURL*          installedPathURL        = [NSURL fileURLWithPath:installedPath];

        NSDictionary*   installedInfoPlist      = (__bridge NSDictionary*)CFBundleCopyInfoDictionaryForURL( (CFURLRef)installedPathURL );
        NSString*       installedBundleVersion  = [installedInfoPlist objectForKey:@"CFBundleVersion"];
        NSInteger       installedVersion        = [installedBundleVersion integerValue];

        NSLog( @"installedVersion: %ld", (long)installedVersion );

        NSBundle*       appBundle       = [NSBundle mainBundle];
        NSURL*          appBundleURL    = [appBundle bundleURL];

        NSURL*          currentHelperToolURL    = [appBundleURL URLByAppendingPathComponent:@"Contents/Library/LaunchServices/com.aaa.azanov.OVPNHelper"];
        NSDictionary*   currentInfoPlist        = (__bridge NSDictionary*)CFBundleCopyInfoDictionaryForURL( (CFURLRef)currentHelperToolURL );
        NSString*       currentBundleVersion    = [currentInfoPlist objectForKey:@"CFBundleVersion"];
        NSInteger       currentVersion          = [currentBundleVersion integerValue];

        NSLog( @"currentVersion: %ld", (long)currentVersion );

        if (installedVersion >= currentVersion)
        {
            result = YES;
            return result;
        }
    }
    else
    {
        NSLog( @"Not installed helper");
    }


        
    AuthorizationItem authItem		= { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
    AuthorizationRights authRights	= { 1, &authItem };
    AuthorizationFlags flags		=	kAuthorizationFlagDefaults				|
    kAuthorizationFlagInteractionAllowed	|
    kAuthorizationFlagPreAuthorize			|
    kAuthorizationFlagExtendRights;
    
    AuthorizationRef authRef = NULL;
    
    // Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper).
    OSStatus status = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);
    if (status != errAuthorizationSuccess)
    {
        NSLog(@"%@", [NSString stringWithFormat:@"Failed to create AuthorizationRef. Error code: %d", (int)status]);
        //[self appendLog:[NSString stringWithFormat:@"Failed to create AuthorizationRef. Error code: %ld", status]];
        
    } else
    {
        // This does all the work of verifying the helper tool against the application
        // and vice-versa. Once verification has passed, the embedded launchd.plist
        // is extracted and placed in /Library/LaunchDaemons and then loaded. The
        // executable is placed in /Library/PrivilegedHelperTools.
        //
        CFErrorRef outError = NULL;
        result = SMJobBless(kSMDomainSystemLaunchd, (__bridge CFStringRef)label, authRef, &outError);
        if (outError)
        {
            NSError *error = (__bridge NSError *)outError;
            NSLog(@"%@", [error localizedDescription]);
            CFRelease(outError);
        }
    }
    
    return result;
}

- (void) create_xpc_connection
{
    if (!xpc_connection_)
    {
        xpc_connection_ = xpc_connection_create_mach_service("com.aaa.azanov.OVPNHelper", NULL, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);

        if (!xpc_connection_)
        {
            [NSException raise:@"OpenVPNConnector" format:@"Failed to create XPC connection"];
        }

        xpc_connection_set_event_handler(xpc_connection_, ^(xpc_object_t event)
        {
            xpc_type_t type = xpc_get_type(event);

            if (type == XPC_TYPE_ERROR) {

                if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
                    NSLog(@"XPC connection interupted.");
                }
                else if (event == XPC_ERROR_CONNECTION_INVALID) {
                    NSLog(@"XPC connection invalid, releasing.");

                }
                else {
                    NSLog(@"Unexpected XPC connection error.");
                }
                xpc_connection_ = nil;

            } else {
                NSLog(@"Unexpected XPC connection event.");
            }
        });

        xpc_connection_resume(xpc_connection_);
    }
}

- (void) executeRootCommand: (NSString *)commandLine
{
    [self create_xpc_connection];

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);

    xpc_dictionary_set_string(message, "cmd", "anycmd");
    xpc_dictionary_set_string(message, "command_line", [commandLine UTF8String]);

    xpc_object_t event = xpc_connection_send_message_with_reply_sync(xpc_connection_, message);
    const char* response = xpc_dictionary_get_string(event, "reply");
    NSLog(@"%@", [NSString stringWithFormat:@"Received response: %s.", response]);
}


- (void) connect
{
    [eventDelegate_ onLog: @"Start connect"];

    if (_configPath == nil)
    {
        [NSException raise:@"OpenVPNConnector" format:@"configPath can't be NULL"];
    }
    if (_username == nil)
    {
        [NSException raise:@"OpenVPNConnector" format:@"username can't be NULL"];
    }
    if (_password == nil)
    {
        [NSException raise:@"OpenVPNConnector" format:@"_password can't be NULL"];
    }
    if (eventDelegate_ == nil)
    {
        [NSException raise:@"OpenVPNConnector" format:@"Neet set OpenVPNConnectorEvents for get events from connector"];
    }
    
    if (currentState_ != STATUS_DISCONNECTED)
    {
        [NSException raise:@"OpenVPNConnector" format:@"already used for connection. Need to disconnect first"];
    }
    
    currentState_ = STATUS_CONNECTING;
   
    // get path to openvpn util
    NSString *pathToOpenVPN = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] bundlePath], @"/Contents/Resources/openvpn"];
    // get path for tun/tap kexts
    NSString *pathToTunKext = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] bundlePath], @"/Contents/Resources/tun.kext"];
    NSString *pathToTapKext = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] bundlePath], @"/Contents/Resources/tap.kext"];
    
    
    [self create_xpc_connection];
    
    NSString *cmdOpenVPN = [NSString stringWithFormat:@"%@ %@ %@ %@ %@ %@ %@ &", pathToOpenVPN, @"--config", _configPath, @"--management", @"127.0.0.1", [NSString stringWithFormat:@"%d",_port], @"--management-query-passwords"];
    NSLog(@"%@", cmdOpenVPN);

    [eventDelegate_ onLog: cmdOpenVPN];
    
    
    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    
    xpc_dictionary_set_string(message, "cmd", "openvpn");
    xpc_dictionary_set_string(message, "openvpncmd", [cmdOpenVPN UTF8String]);
    xpc_dictionary_set_string(message, "path_tun", [pathToTunKext UTF8String]);
    xpc_dictionary_set_string(message, "path_tap", [pathToTapKext UTF8String]);
    
    
    xpc_connection_send_message_with_reply(xpc_connection_, message, dispatch_get_main_queue(), ^(xpc_object_t event)
    {
        const char* response = xpc_dictionary_get_string(event, "reply");
        NSLog(@"%@", [NSString stringWithFormat:@"Received response: %s.", response]);
        [eventDelegate_ onLog: [NSString stringWithFormat:@"Received response: %s.", response]];
    });
    
    bStopThread_ = false;
    bSockConnected_ = false;
    thread_ = [[NSThread alloc] initWithTarget:self selector:@selector(controlOpenVPNThread) object:nil];
    [thread_ start];
}

- (void) disconnect
{
    if (sock_ != -1 && bSockConnected_)
    {
        char *message = "signal SIGTERM\n";
        [self sendAll:message :strlen(message)];
    }
    if (sock_ != -1)
    {
        close(sock_);
        sock_ = -1;
    }
    bStopThread_ = true;
}

-(enum CONNECTION_STATUS)getConnectionStatus
{
    return currentState_;
}

-(int) sendAll: (char *)buf : (ssize_t)len
{
    @synchronized(self)
    {
        ssize_t total = 0;
        ssize_t l = len;
        ssize_t bytesleft = l;
        ssize_t n = 0;
        while(total < l)
        {
            n = send(sock_, buf+total, bytesleft, 0);
            if (n == -1)
            {
                NSLog(@"send to socket failed");
                break;
            }
            total += n;
            bytesleft -= n;
        }
        return n==-1?-1:0; // return -1 on failure, 0 on success
    }
}

-(int)sgetline: (int)sock : (char *) outbuf
{
    int bytesloaded = 0;
    ssize_t ret;
    char buf;
    
    do
    {
        // read a single byte
        ret = read(sock, &buf, 1);
        if (ret < 1)
        {
            // error or disconnect
            return -1;
        }
        
        outbuf[bytesloaded] = buf;
        bytesloaded++;
        
        // has end of line been reached?
        if (buf == '\n')
            break; // yes
        
    } while (1);
    
    outbuf[bytesloaded - 1] = '\0';
    return bytesloaded; // number of bytes in the line, not counting the line break
}

-(void)controlOpenVPNThread
{
    NSLog(@"controlOpenVPNThread started");
    int numberOfConnectRetries = 0;
    char server_reply[10000];
    bool bStateModeOn = false;
    bool bProxyAuthErrorEmited = false;
    
    
    while (!bStopThread_)
    {
        if (!bSockConnected_)
        {
            sock_ = socket(AF_INET, SOCK_STREAM, 0 );
            if (sock_ == -1)
            {
                [NSException raise:@"OpenVPNConnector" format:@"cannot create socket"];
                currentState_ = STATUS_DISCONNECTED;
                return;
            }
            
            struct sockaddr_in serv_addr;
            bzero(&serv_addr, sizeof(serv_addr));
            serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(_port);
            
            if (connect(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
            {
                close(sock_);
                numberOfConnectRetries++;
                
                if (numberOfConnectRetries > 1000)
                {
                    NSLog(@"Can't connect to openvpn socket after 1000 retries");
                    currentState_ = STATUS_DISCONNECTED;
                    [eventDelegate_ onError: MAC_NO_OPENVPN_SOCKET];
                    return;
                }
            }
            else
            {
                bSockConnected_ = true;
            }
        }
        
        if (bSockConnected_)
        {
            if (!bStateModeOn)
            {
                char *message = "state on all\n";
                [self sendAll:message :strlen(message)];
                
                char *message2 = "log on\n";
                [self sendAll:message2 :strlen(message2)];

                char *message3 = "bytecount 1\n";
                [self sendAll:message3 :strlen(message3)];

                bStateModeOn = true;
            }
            
            //Receive a reply line from the server
            ssize_t ret = [self sgetline:sock_: server_reply];
            if(ret < 0)
            {
                break;
            }
            
            NSString *strServerReply = [NSString stringWithUTF8String:server_reply];
            
            NSLog(@"%@", strServerReply);
            
            if ([strServerReply rangeOfString:@"PASSWORD:Need 'Auth' username/password"].location != NSNotFound)
            {
                char *message[1024];
                sprintf((char *)message, "username \"Auth\" %s\n", [_username UTF8String]);
                [self sendAll:(char *)message :strlen((char *)message)];
                sprintf((char *)message, "password \"Auth\" %s\n", [_password UTF8String]);
                [self sendAll:(char *)message :strlen((char *)message)];
            }
            else if ([strServerReply rangeOfString:@"PASSWORD:Need 'HTTP Proxy' username/password"].location != NSNotFound)
            {
                char *message[1024];
                sprintf((char *)message, "username \"HTTP Proxy\" %s\n", [_proxyUsername UTF8String]);
                [self sendAll:(char *)message :strlen((char *)message)];
                sprintf((char *)message, "password \"HTTP Proxy\" %s\n", [_proxyPassword UTF8String]);
                [self sendAll:(char *)message :strlen((char *)message)];
            }
            else if ([strServerReply rangeOfString:@"PASSWORD:Verification Failed: 'Auth'"].location != NSNotFound)
            {
                [eventDelegate_ onError: MAC_AUTH_ERROR];
            }
            else if ([strServerReply rangeOfString:@"FATAL:Cannot allocate TUN/TAP dev dynamically"].location != NSNotFound)
            {
                [eventDelegate_ onError: MAC_CANNOT_ALLOCATE_TUN_TAP];
            }
            else if ([strServerReply rangeOfString:@"Proxy requires authentication"].location != NSNotFound)
            {
                if (!bProxyAuthErrorEmited)
                {
                    [eventDelegate_ onError: MAC_PROXY_AUTH_ERROR];
                    bProxyAuthErrorEmited = true;
                }
            }
            else if ([strServerReply rangeOfString:@">BYTECOUNT:"].location != NSNotFound)
            {
                NSArray *pars = [strServerReply componentsSeparatedByString: @":"];
                if (pars.count > 1)
                {
                    NSArray *pars2 = [pars[1] componentsSeparatedByString: @","];
                    if (pars2.count == 2)
                    {
                        long l1 = [pars2[0] longLongValue];
                        long l2 = [pars2[1] longLongValue];
                        [eventDelegate_ onStats: l1: l2];
                    }
                }

            }
            else if ([strServerReply rangeOfString:@">STATE:"].location != NSNotFound)
            {
                NSArray *pars = [strServerReply componentsSeparatedByString: @","];
                [eventDelegate_ onStateChanged: pars[1]];
                
                if ([strServerReply rangeOfString:@"CONNECTED,SUCCESS"].location != NSNotFound)
                {
                    currentState_ = STATUS_CONNECTED;
                    [eventDelegate_ onConnected];
                }
            }
            else if ([strServerReply rangeOfString:@">LOG:"].location != NSNotFound)
            {
                //NSArray *pars = [strServerReply componentsSeparatedByString: @","];
                //[eventDelegate_ onLog: pars[2]];
                [eventDelegate_ onLog: strServerReply];

            }
        }
        
        [NSThread sleepForTimeInterval:0.001];
    }
    NSLog(@"controlOpenVPNThread stopped");
    bSockConnected_ = false;
    if (currentState_ != STATUS_DISCONNECTED)
    {
        [eventDelegate_ onDisconnected];
        currentState_ = STATUS_DISCONNECTED;
    }
}

@end
