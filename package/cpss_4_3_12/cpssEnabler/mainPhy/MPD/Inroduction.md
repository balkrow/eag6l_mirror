# MPD overview

Marvell PHY Driver (MPD) is a software package that enables customer application to configure and manage the commonly used functions in a Marvell PHY.

MPD architecture is modular and consists of two layers, the API layer which is an abstract, common to all PHY driver implementations and the implementation layer

MPD is an OS-independent software package, it does not call OS APIs, it expects to receive the OS APIs implementations as callbacks in the initialization process.

# Key features

- **PHY initialization** – MPD implements PHY initialization sequence per PHY datasheet/release notes requirements.
- **Commonly used configuration** – MPD provides API for speed, duplex, negotiation, administrative disable (shutdown), EEE, MDI/X and various status indications.
- **PHY FW loading** – on requiring PHYs, MPD will load the PHY FW.

# Architecture

Component hierarchy

![](hirarchy.png)

### Application

The application is the user application, it uses MPD in order to initialize and configure PHY and it provides the OS and External services implementations

### MPD

MPD is the API layer, it provides APIs for initialization and a single function API for setting and retrieving data from PHY. The action will be determined based on the values passed to this single API function.

### Driver Layer

This layer (which is depicted as MTD, AlaskaC, Alaska ext…) contains the specific PHYs implementation, this layer is the one that has the logic and the &quot;know how&quot; in order to correctly configure PHY HW.

### Operating System

Application must provide MPD with the following OS services:

- Sleep
- Memory (alloc, free, realloc)

For example, when MPD requests memory allocation, it calls a function that was configured by the application upon MPD initialization. This function pointer connects MPD to the required OS service.

### External Services

These services connect MPD to the HW and to the application

Application Must provide the following:

- MDIO access routines
- Failure treatment routing

Application may provide the flowing:

- Debug services
- Transceiver enable/disable
- Invert MDI/MDIX status
- Get administrative status
- Get FW files – in order to load PHY FW

# MPD initialization

MPD initialization is done in three stages, please refer to mpdSample.c for an implementation example of this sequence

### Stage 1 - mpdDriverInitDb

This function will:

- Store the provided user callbacks
- Allocate global MPD db
- Bind to debug services (if debug callbacks were provided)

### Stage 2 – mpdPortDbUpdate

In this function port DB will be updated, port entries (in DB) are allocated at this stage.

In this stage caller will provide per port the following information:

- The device (packet processor) through which data path is connected. Set to 0 if not relevant
- The port (in packet processor) through which data path is connected. Set to 0 if not relevant
- PHY number, unique within MDIO BUS
- PHY type
- Transceiver type – copper/fiber/combo
- Port location in multi-port PHY
- MDIO information:
  - Device (through which MDIO is connected)
  - Bus
  - Address
  - Read and Write functions – in addition to global MDIO access function, application may provide port specific access function
- Additional PHY/Port specific settings including MDI/X inversion, Short reach Serdes, lane swap and TX Serdes (non-default) settings

### Stage 3 – mpdDriverInitHw

This is the final stage of init, in this stage

- The per PHY type DB is created (only for PHY types defined in previous stage)
- PHY (HW) is initialized
- Where required, PHY FW is loaded and verified as running

After this stage, PHY &amp; MPD are ready to receive configuration &quot;operations&quot;

# How to configure PHY
After initialization (see mpdSampleInit in mpdSample.c), Application will configure PHY using a single API function

```C
extern MPD_RESULT_ENT mpdPerformPhyOperation(
	
	IN UINT_32 rel_ifIndex,
	
	IN MPD_OP_CODE_ENT op,
	
	INOUT MPD_OPERATIONS_PARAMS_UNT * params_PTR
);
```

rel_ifindex is the unique numeric identifier assigned to port during the initialization phase
op is the operation/action that will be performed
params_PTR is the data that is either being passed or retrieved from MPD as part of operation exeution
### Example:
```C
UINT_32 					rel_ifIndex;
MPD_RESULT_ENT 				result;
MPD_OPERATIONS_PARAMS_UNT 	phyParams;

/* read link status on port 1 */
memset(&phyParams , 0 , sizeof(phyParams));

result = mpdPerformPhyOperation(1,
	MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
	& phyParams);
	
	
if ( result == MPD_OK_E ) {
	printf("rel_ifIndex [%d] reports link [%s] negotiation is [%s Complete]\n",	
	1,	
	((phyParams.phyInternalOperStatus.isOperStatusUp)?"UP" :"Down"),	
	phyParams.phyInternalOperStatus.isAnCompleted?"" :"Not");
	
}
else {
	printf("read internal statuus on rel_ifIndex [%d] failed\n", 1);
}
```
