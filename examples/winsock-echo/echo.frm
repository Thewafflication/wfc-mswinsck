VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form frmEcho
   Caption         =   "Winsock echo on the loopback address"
   ClientHeight    =   3300
   ClientWidth     =   6000
   Begin MSWinsockLib.Winsock wsListen
      Left            =   120
      Top             =   120
   End
   Begin MSWinsockLib.Winsock wsClient
      Left            =   720
      Top             =   120
   End
   Begin MSWinsockLib.Winsock wsServer
      Left            =   1320
      Top             =   120
   End
   Begin VB.ListBox lstLog
      Height          =   2940
      Left            =   120
      Top             =   120
      Width           =   5760
   End
End
Attribute VB_Name = "frmEcho"
Option Explicit

Private Sub Form_Load()
    lstLog.AddItem "Winsock control on the loopback address"
    wsListen.LocalPort = 0
    wsListen.Listen
    lstLog.AddItem "Server listening on port " & wsListen.LocalPort
    wsClient.Connect "127.0.0.1", wsListen.LocalPort
End Sub

Private Sub wsListen_ConnectionRequest(ByVal requestID As Long)
    wsServer.Accept requestID
    lstLog.AddItem "Server accepted connection " & requestID
End Sub

Private Sub wsClient_Connect()
    lstLog.AddItem "Client connected"
    wsClient.SendData "hello from WFC"
End Sub

Private Sub wsServer_DataArrival(ByVal bytesTotal As Long)
    Dim s As String
    wsServer.GetData s
    lstLog.AddItem "Server received: " & s
    wsServer.SendData UCase$(s)
End Sub

Private Sub wsClient_DataArrival(ByVal bytesTotal As Long)
    Dim s As String
    wsClient.GetData s
    lstLog.AddItem "Client received: " & s
    wsClient.Close
End Sub

Private Sub wsServer_Close()
    lstLog.AddItem "Server saw the client close"
    wsServer.Close
End Sub
