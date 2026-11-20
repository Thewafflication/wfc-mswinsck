VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form frmEcho
   BackColor       =   &H001F1411&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Winsock echo"
   ClientHeight    =   5370
   ClientWidth     =   7200
   MaxButton       =   0   'False
   StartUpPosition =   2  'CenterScreen
   Begin MSWinsockLib.Winsock wsListen
      Left            =   0
      Top             =   0
   End
   Begin MSWinsockLib.Winsock wsClient
      Left            =   0
      Top             =   0
   End
   Begin MSWinsockLib.Winsock wsServer
      Left            =   0
      Top             =   0
   End
   Begin VB.Label lblStatus
      BackColor       =   &H001F1411&
      Caption         =   "Starting"
      ForeColor       =   &H0054B4FF&
      BeginProperty Font 
         Name            =   "Segoe UI"
         Size            =   10.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   330
      Left            =   360
      Top             =   4830
      Width           =   6480
   End
   Begin VB.Label lblLog
      BackColor       =   &H0033201B&
      ForeColor       =   &H00F5EBE8&
      BeginProperty Font 
         Name            =   "Segoe UI"
         Size            =   11
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2700
      Left            =   570
      Top             =   1530
      Width           =   6060
   End
   Begin VB.Label lblSubtitle
      BackColor       =   &H001F1411&
      Caption         =   "A server and a client talk on the loopback address."
      ForeColor       =   &H00BEA098&
      BeginProperty Font 
         Name            =   "Segoe UI"
         Size            =   10.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   360
      Top             =   750
      Width           =   6480
   End
   Begin VB.Label lblTitle
      BackColor       =   &H001F1411&
      Caption         =   "Winsock echo"
      ForeColor       =   &H00C6D63D&
      BeginProperty Font 
         Name            =   "Segoe UI"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   360
      Top             =   240
      Width           =   6480
   End
   Begin VB.Shape shpCard
      BackStyle       =   1  'Opaque
      BorderColor     =   &H006B4A40&
      FillColor       =   &H0033201B&
      FillStyle       =   0  'Solid
      Height          =   3120
      Left            =   360
      Top             =   1320
      Width           =   6480
   End
   Begin VB.Shape shpShadowNear
      BackStyle       =   1  'Opaque
      BorderStyle     =   0  'Transparent
      FillColor       =   &H000F0A08&
      FillStyle       =   0  'Solid
      Height          =   3120
      Left            =   405
      Top             =   1380
      Width           =   6480
   End
   Begin VB.Shape shpShadowFar
      BackStyle       =   1  'Opaque
      BorderStyle     =   0  'Transparent
      FillColor       =   &H0017100D&
      FillStyle       =   0  'Solid
      Height          =   3120
      Left            =   465
      Top             =   1455
      Width           =   6480
   End
End
Attribute VB_Name = "frmEcho"
Option Explicit

Private mSteps As Long

Private Sub Form_Load()
    Log "Winsock control on the loopback address"
    wsListen.LocalPort = 0
    wsListen.Listen
    Log "Server listening on port " & wsListen.LocalPort
    lblStatus.Caption = "Listening on 127.0.0.1:" & wsListen.LocalPort
    wsClient.Connect "127.0.0.1", wsListen.LocalPort
End Sub

Private Sub Log(ByVal text As String)
    mSteps = mSteps + 1
    lblLog.Caption = lblLog.Caption & Format$(mSteps, "00") & "   " & text & vbCrLf
End Sub

Private Sub wsListen_ConnectionRequest(ByVal requestID As Long)
    wsServer.Accept requestID
    Log "Server accepted the connection"
End Sub

Private Sub wsClient_Connect()
    Log "Client connected"
    wsClient.SendData "hello from WFC"
    Log "Client sent: hello from WFC"
End Sub

Private Sub wsServer_DataArrival(ByVal bytesTotal As Long)
    Dim s As String
    wsServer.GetData s
    Log "Server received: " & s
    wsServer.SendData UCase$(s)
End Sub

Private Sub wsClient_DataArrival(ByVal bytesTotal As Long)
    Dim s As String
    wsClient.GetData s
    Log "Client received: " & s
    wsClient.Close
End Sub

Private Sub wsServer_Close()
    Log "Server saw the client close"
    wsServer.Close
    lblStatus.ForeColor = &HC6D63D
    lblStatus.Caption = "Done: the message came back in capitals"
End Sub
