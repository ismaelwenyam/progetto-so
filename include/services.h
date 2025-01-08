#ifndef SERVICES_H
#define SERVICES_H

#define NUMBER_OF_SERVICES 6

#define IRP "irp"		/* Invio e ritiro pacchi */
#define ILR "ilr"		/* Invio e ritiro lettere e raccomandate */
#define PVB "pvb"		/* Prelievi e versamenti bancoposta */
#define PBP "pbp"		/* Pagamento bolletini postali */
#define APF "apf"		/* Acquisto prodotti finanziari */
#define AOB "aob"		/* Acquisto orologi e braccialetti */

#define SERVICE_LEN 4

typedef struct servizio ServizioAdt, *ServizioAdtPtr;
struct servizio {
        char name[SERVICE_LEN];
        int temp;
        short available;
};
#endif
