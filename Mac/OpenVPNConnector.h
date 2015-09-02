//
//  OpenVPNConnector.h
//  OVPNClient
//

#import <Foundation/Foundation.h>

// need make subclass for OpenVPNConnectorEvents and implement methods for get events from OpenVPNConnector class
@protocol OpenVPNConnectorEvents

// OpenVPN errors
enum ERROR {AUTH_ERROR, NO_OPENVPN_SOCKET, CANNOT_ALLOCATE_TUN_TAP};

enum CONNECTION_STATUS {STATUS_DISCONNECTED, STATUS_CONNECTING,  STATUS_CONNECTED};

-(void)onConnected;
-(void)onDisconnected;
-(void)onStateChanged: (NSString *) state;
-(void)onError: (enum ERROR) error;
-(void)onLog: (NSString *)logStr;
-(void)onStats: (long)bytesIn: (long)bytesOut;

@end


@interface OpenVPNConnector : NSObject
{
    id <OpenVPNConnectorEvents> eventDelegate_;
}

@property NSString *configPath;    // path to *.ovpn file
@property NSString *username;
@property NSString *password;
@property int       port;           // port for network exchange with openvpn process (default value 9544)

- (void) setEventDelegate: (id <OpenVPNConnectorEvents> )delegate;
- (BOOL) installHelper: (NSString *)label;
- (void) connect;
- (void) disconnect;
- (void) executeRootCommand: (NSString *)commandLine;
-(enum CONNECTION_STATUS)getConnectionStatus;

@end
