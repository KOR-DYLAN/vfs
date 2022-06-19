#ifndef __VFAT_INTERFACE__H__
#define __VFAT_INTERFACE__H__

#define DEFAULT_LOGICAL_SECTOR_SIZE     (512)

# define MBR_GENERIC_BOOTSTRAP_LEN      (446)
# define MBR_GENERIC_PATITION_LEN       (16)
# define MBR_GENERIC_NUM_OF_PATITION    (4)

#define PART_ID_NTFS                    (0x07)
#define PART_ID_exFAT                   (0x07)
#define PART_ID_FAT32_CHS               (0x0B)
#define PART_ID_FAT32_LBA               (0x0C)
#define PART_ID_LINUX                   (0x83)

#define MBR_SIGNATURE                   (0xAA55)

union __tag_master_boot_record
{
    uint8_t mem[DEFAULT_LOGICAL_SECTOR_SIZE];
    struct __attribute__((__packed__))
    {
        uint8_t bootstrap[MBR_GENERIC_BOOTSTRAP_LEN];

        struct 
        {
            uint32_t bootflag   : 8;
            uint32_t chs_start  : 24;
            uint32_t type       : 8;
            uint32_t chs_end    : 24;
            uint32_t lba_start;
            uint32_t num_of_sectors;
        } partition[MBR_GENERIC_NUM_OF_PATITION];
        uint16_t signature; /* 0x55AA */
    } generic;
};
typedef union __tag_master_boot_record mbr_t;

struct __tag_bios_parameter_block
{
    uint16_t    bytes_per_sector;               /* offset: 0x0B,  2 byte - Byters per Sector 
                                                    : 한 섹터 당 할당되는 byte 크기 */
    uint8_t     sector_per_cluster;             /* offset: 0x0D,  1 byte - Sector per Cluster 
                                                    : 한 클러스터가 가지는 섹터의 수( 최대 클러스터 크기는 32KB ) */
    uint16_t    reserved_sector_count;          /* offset: 0x0E,  2 byte - Reserved Sector Count 
                                                    : FAT Area가 나오기 전에 예약된 영역의 섹터 수( FAT32 = 32 sector ) */
    uint8_t     num_of_fat_tables;              /* offset: 0x10,  1 byte - Num of FAT Tables
                                                    : FAT Aread의 개수, 대부분 FAT #1, #2가 존재(default로 backup 제공) */
    uint16_t    root_direcotry_entry_count;     /* offset: 0x11,  2 byte - Root Directory Entry Count
                                                    : FAT12/16의 root directory에 있을 수 있는 최대 파일의 개수 */
    uint16_t    total_sector16;                 /* offset: 0x13,  2 byte - Total Sector 16
                                                    : 파일시스템에 있는 총 섹터의 수( 볼륨상의 총 섹터 수 ) */
    uint8_t     media_type;                     /* offset: 0x15,  1 byte - Media Type
                                                    : 볼륨에 어떤 미디어가 저장되어 있는지 나타냄( 0xF8 except floopy disk ) */
    uint16_t    fat_size16;                     /* offset: 0x16,  2 byte - FAT Size 16
                                                    : FAT12/16의 FAT Area의 섹터 수( FAT32 = 0 ) */
    uint16_t    sector_per_track;               /* offset: 0x18,  2 byte - Sector per Track
                                                    : 저장 장치의 트랙 당 섹터의 수( 0x3F = 63 ) */
    uint16_t    num_of_heads;                   /* offset: 0x1A,  2 byte - Number of Heads
                                                    : 장치의 헤더 수 */
    uint32_t    hidden_sector;                  /* offset: 0x1C,  4 byte - Hidden Sectors
                                                    : 파티션 시작 전 섹터의 수( 볼륨 앞에 숨겨진 섹터의 수 ) */
    uint32_t    total_sector32;                 /* offset: 0x20,  4 byte - Total Sector 32
                                                    : 파일시스템에 있는 총 섹터의 수( Total Sector 16으로 설정 불가능 한 sector 개수일 경우 여기에 설정 ) */
    uint32_t    fat_size32;                     /* offset: 0x24,  4 byte - FAT Size 32
                                                    : FAT 영역의 섹터 수 */
    uint16_t    ext_flags;                      /* offset: 0x28,  2 byte - Ext Flags
                                                    : 여러 개의 FAT 영역을 사용 할 경우 설정 값 표시 */
    uint16_t    file_system_version;            /* offset: 0x2A,  2 byte - File System Version
                                                    : 파일 시스템의 주 버전(major), 부 버전(minor) 번호 */
    uint32_t    root_directory_cluster;         /* offset: 0x2C,  4 byte - Root Directory Cluster
                                                    : root directory가 존재하는 cluster의 위치(FAT32: 유동, FAT12/16: 고정) */
    uint16_t    file_system_information_sector; /* offset: 0x30,  2 byte - File System Information
                                                    : FSINFO 구조체가 있는 섹터의 위치( 보통 0x01 ) */
    uint16_t    backup_boot_record_sector;      /* offset: 0x32,  2 byte - Backup Boot Record
                                                    : 백업 된 boot sector의 위치( 보통 0x06 ) */
    uint8_t     reserved1[12];                  /* offset: 0x34,  12 byte - Reserved */
    uint8_t     drive_number;                   /* offset: 0x40,  1 byte - Drive Number
                                                    : 드라이브 번호 */
    uint8_t     reserved2;                      /* offset: 0x41,  1 byte - Reserved */
    uint8_t     boot_signature;                 /* offset: 0x42,  1 byte - Boot Signature
                                                    : 이 여역이 존재 할 경우 아래의 추가 데이터 존재( 보통 0x29 ) */
    uint8_t     volume_id[4];                   /* offset: 0x43,  4 byte - Volume ID
                                                    : Volume Serial Number */
    uint8_t     volume_lable[11];               /* offset: 0x47,  11 byte - Volume Label
                                                    : 값이 없으면 "NO NAME" 으로 적힘 */
    uint8_t     file_system_type[8];            /* offset: 0x52,  8 byte - File System Type
                                                    : 파일 시스템 형식 표시( 보통 FAT32로 표시 ) */
} __attribute__((__packed__));
typedef struct __tag_bios_parameter_block BPP_t;

union __tag_partition_boot_record   /* alias of reserved area : 0, 6 sector*/
{
    uint8_t mem[DEFAULT_LOGICAL_SECTOR_SIZE];
    struct __attribute__((__packed__))
    {
        uint8_t     boot_code[3];                                   /* offset: 0x00,  3 byte - Jump Command to Boot Code */
        uint8_t     oem_id[8];                                      /* offset: 0x03,  8 byte - OEM(Original Equipment Manufacturing) ID  */
        BPP_t       bpp;                                            /* offset: 0x0B, 79 byte - BPB(BIOS Parameter Block)  */
        char        err_message[420];                               /* offset: 0x5A, 420 byte - Boot Code Error Message  */
        uint16_t    signature;                                      /* offset: 0x1FE, 2 byte - FAT32 File System VBR Area Signature( 0xAA55 )  */
    } field;
};
typedef union __tag_partition_boot_record pbr_t;

union __tag_file_system_information   /* alias of reserved area : 1, 7 sector*/
{
    uint8_t mem[DEFAULT_LOGICAL_SECTOR_SIZE];
    struct
    {
        uint32_t    signature1;             /* offset: 0x00, 4 byte - Signature(0x52526141)
                                                : Little Endian(0x41615252) */
        uint8_t     reserved1[480];         /* offset: 0x04, 480 byte - reserved */
        uint32_t    signature2;             /* offset: 0x1E4, 4 byte - Signature(0x72724161)
                                                : Little Endian(0x61417272) */
        uint32_t    num_of_free_clusters;   /* offset: 0x1E8, 4 byte - Num of Free Clusters
                                                : 비할당 클러스터 수( 사용 가능 한 클러스터의 수 ) */
        uint32_t    next_free_cluster;      /* offset: 0x1EC, 4 byte - Next Free Cluster
                                                : 다음 비할당 클러스터( 사용 가능 한 클러스터 시작 위치 ) */
        uint8_t     reserved2[14];          /* offset: 0x1F0, 14 byte - reserved */
        uint16_t    signature3;             /* offset: 0x1FD, 2 byte - Signature(0x55AA)
                                                : Little Endian(0xAA55) */
    } field;
};
typedef union __tag_file_system_information FSINFO_t;

void vfat_parse_mbr(int fd);

#endif  //!__VFAT_INTERFACE__H__
