<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <!--now parse the whole thing-->
    <xsl:template match="/*">
        <html>
            <head>
                <!--<link rel="stylesheet" type="text/css" href="HWStyle.css"/>-->
            </head>
            <body>
                <h3>HWDescription</h3>
                <xsl:apply-templates select="BeBoard"/>
            </body>
        </html>
    </xsl:template>

    <!--Template for BeBoard nodes-->
    <xsl:template match="BeBoard">
        <h4> BeBoard Id: <xsl:value-of select="@Id"/> </h4>
        <div class="BeBoard">
            <ul>
                <li>BoardType: <xsl:value-of select="@boardType"/></li>
                <li>EventType: <xsl:value-of select="@eventType"/></li>
            </ul>
            <table border="0">
              <tr>
                <td>ConnectionId: <xsl:value-of select="connection/@id"/></td>
                <td>URI: <xsl:value-of select="connection/@uri"/></td>
                <td>Address Table: <xsl:value-of select="connection/@address_table"/></td>
              </tr>
            </table>
        
            <!--Module-->
            <xsl:apply-templates select="Module"/>
            <!--SLINK-->
            <xsl:apply-templates select="SLink"/>

            <!--Registers-->
            <div class="BeBoardRegisters">
                <h4>Registers</h4>
                <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
            </div>
        </div>
    </xsl:template>

    <!--Template for Module nodes-->
    <xsl:template match="Module">
        <h4> Module Id: <xsl:value-of select="@FeId"/></h4>
        <div class="Module">
            FMCId: <xsl:value-of select="@FMCId"/>
             ModuleId: <xsl:value-of select="@ModuleId"/>
             Status: <xsl:value-of select="@Status"/>
            <br></br>
            CBC FilePath: <xsl:value-of select="CBC_Files/@path"/>
            <xsl:apply-templates select="Global"/>
            <ul>
            <xsl:apply-templates select="Global_CBC_Register"/>
            </ul>
            <ol>
                <xsl:apply-templates select="CBC"/>
            </ol>
        </div>
    </xsl:template>

    <!--Template for Global|CBC Settings node-->
    <xsl:template match="Settings">
        <li> Settings: threshold=<xsl:value-of select="@threshold"/> latency=<xsl:value-of select="@latency"/></li>
    </xsl:template>

    <!--Template for Global|CBC TestPulse node-->
    <xsl:template match="TestPulse">
        <li> TestPulse: enable=<xsl:value-of select="@enable"/> ploarity=<xsl:value-of select="@polarity"/> amplitude=<xsl:value-of select="@amplitude"/> channelgroup=<xsl:value-of select="@channelgroup"/> delay=<xsl:value-of select="@delay"/> groundOthers=<xsl:value-of select="@groundothers"/></li>
    </xsl:template>

    <!--Template for Global|CBC ClusterStub node-->
    <xsl:template match="ClusterStub">
        <li> ClusterStub: clusterwidth=<xsl:value-of select="@clusterwidth"/> ptwidth=<xsl:value-of select="@ptwidth"/> layerswap=<xsl:value-of select="@layerswap"/> off1=<xsl:value-of select="@off1"/>
         off2=<xsl:value-of select="@off2"/> off3=<xsl:value-of select="@off3"/> off4=<xsl:value-of select="@off4"/></li>
    </xsl:template>

    <!--Template for Global|CBC Misc node-->
    <xsl:template match="Misc">
         <li> Misc: analogmux=<xsl:value-of select="@analogmux"/>
         <!--<xsl:if test="@pipelogic"/>-->
         pipelogic=<xsl:value-of select="@pipelogic"/> stublogic=<xsl:value-of select="@stublogic"/> or254=<xsl:value-of select="@or254"/> tpgclock=<xsl:value-of select="@tpgclock"/> testclock=<xsl:value-of select="@testclock"/> dll=<xsl:value-of select="@dll"/></li>
                    <!--</xsl:if>-->
    </xsl:template>

    <!--Template for Global|CBC ChannelMask node-->
    <xsl:template match="ChannelMask">
        <li>ChannelMask: <xsl:value-of select="@disable"/></li>
    </xsl:template>

    <!--Template for GlobalNodes-->
    <xsl:template match="Global">
        <h5> Global CBC Config </h5>
        <div class="Global">
            <ul>
                <xsl:apply-templates select="Settings"/>
                <xsl:apply-templates select="TestPulse"/>
                <xsl:apply-templates select="ClusterStub"/>
                <xsl:apply-templates select="Misc"/>
                <xsl:apply-templates select="ChannelMask"/>
            </ul>
        </div>
    </xsl:template>

    <!--Template for GlobalCBCRegister node-->
    <xsl:template match="Global_CBC_Register">
        <li>Global CBC Register: <xsl:value-of select="@name"/> : <xsl:value-of select="."/></li>
    </xsl:template>

    <!--Template for CBC nodes-->
    <xsl:template match="CBC">
        <li> CBC Id: <xsl:value-of select="@Id"/>  ConfigFile: <xsl:value-of select="@configfile"/></li>
        <ul>
            <xsl:apply-templates select="Settings"/>
            <xsl:apply-templates select="TestPulse"/>
            <xsl:apply-templates select="ClusterStub"/>
            <xsl:apply-templates select="Misc"/>
            <xsl:apply-templates select="ChannelMask"/>
            <xsl:if test="Register">
            <li> CBCRegisters: </li>
            <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
            </xsl:if>
        </ul>

    </xsl:template>

    <!--Template for SLINK node-->
    <xsl:template match="SLink">
    <h4>SLink</h4>
    <div class="SLink">
       Debug Mode: <xsl:value-of select="DebugMode/@type"/>
       <br></br>
       ConditionData
       <ul>
            <xsl:apply-templates match="ConditinData"/>
       </ul>
    </div>
    </xsl:template>

    <!--Template for SLINKConditionData node-->
    <xsl:template match="ConditionData">
               <li>
                   Type: <xsl:value-of select="@type"/>
                   <xsl:if test="@type = 'I2C'">
                   <td> Register: <xsl:value-of select="@Register"/></td>
                   <td> FeId: <xsl:value-of select="@FeId"/></td>
                   <td> CbcId: <xsl:value-of select="@CbcId"/></td>
                   </xsl:if>
                   <xsl:if test="@type = 'User'">
                   <td> UID: <xsl:value-of select="@UID"/></td>
                   <td> FeId: <xsl:value-of select="@FeId"/></td>
                   <td> CbcId: <xsl:value-of select="@CbcId"/></td>
                   <td> Value: <xsl:value-of select="."/></td>
                   </xsl:if>
                   <xsl:if test="@type = 'HV'">
                   <td> FeId: <xsl:value-of select="@FeId"/></td>
                   <td> Sensor: <xsl:value-of select="@Sensor"/></td>
                   <td> Value: <xsl:value-of select="."/></td>
                   </xsl:if>
                   <xsl:if test="@type = 'TDC'">
                   <td> FeId: <xsl:value-of select="@FeId"/></td>
                   </xsl:if>
               </li>
   </xsl:template>
    

    <!--Template for RegisterNode with children-->
    <xsl:template match="Register[Register]">
        <li><xsl:value-of select="@name"/></li>
            <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
    </xsl:template>

    <!--Template for Register Node without children-->
    <xsl:template match="Register[not(Register)]">
        <li><xsl:value-of select="@name"/>: <xsl:value-of select="."/></li>
    </xsl:template>

</xsl:stylesheet>
