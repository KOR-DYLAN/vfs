#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "lib/vfat/vfat_interface.h"

static char vfat_get_ascii(uint8_t val)
{
    return ((0x1F < val) && (val < 0x7F)) ? (char)val : '.';
}

static void vfat_dump(const char* msg, uint8_t* buf, size_t len)
{
    size_t i, j;
    uint8_t* pos;

    printf(" > %s\n", msg);
    for(i = 0; i < len; i += 16)
    {
        pos = &buf[i];
        printf("%08x : ", (uint32_t)(buf + i));

        for(j = 0; j < 16; j++)
        {
            printf("%02x ", pos[j]);
            if(len <= (i + j + 1))
            {
                break;
            }
        }

        while(j < 16)
        {
            printf("   ");
            j++;
        }
        printf("| ");

        for(j = 0; j < 16; j++)
        {
            printf("%c ", vfat_get_ascii(pos[j]));
            if(len <= (i + j + 1))
            {
                break;
            }
        }
        puts("");
    }
}

void vfat_parse_mbr(int fd)
{
    size_t i;
    off_t ret;
    mbr_t mbr_buf;
    pbr_t pbr_buf;
    FSINFO_t fsinfo_buf;

    ret = read(fd, &mbr_buf, sizeof(mbr_t));
    if(ret == -1)
    {
        perror("mmc read failed...\n");
        return;
    }

    for(i = 0 ; i < 4; i++)
    {
        printf(">> partition[%lu]\n", i);
        printf("   - bootflag       : %x\n", mbr_buf.generic.partition[i].bootflag);
        printf("   - chs_start      : %x\n", mbr_buf.generic.partition[i].chs_start);
        printf("   - type           : %x\n", mbr_buf.generic.partition[i].type);
        printf("   - chs_end        : %x\n", mbr_buf.generic.partition[i].chs_end);
        printf("   - lba_start      : %x\n", mbr_buf.generic.partition[i].lba_start);
        printf("   - num_of_sectors : %x\n", mbr_buf.generic.partition[i].num_of_sectors);
        printf(" > partition size   : sector_size x num_of_sectors = %f [byte]\n", (float)DEFAULT_LOGICAL_SECTOR_SIZE * mbr_buf.generic.partition[i].num_of_sectors);
        printf(" > partition size   : sector_size x num_of_sectors = %f [KB]\n", (float)DEFAULT_LOGICAL_SECTOR_SIZE * mbr_buf.generic.partition[i].num_of_sectors / 1024);
        printf(" > partition size   : sector_size x num_of_sectors = %f [MB]\n", (float)DEFAULT_LOGICAL_SECTOR_SIZE * mbr_buf.generic.partition[i].num_of_sectors / 1024 / 1024);
        printf(" > partition size   : sector_size x num_of_sectors = %f [GB]\n", (float)DEFAULT_LOGICAL_SECTOR_SIZE * mbr_buf.generic.partition[i].num_of_sectors / 1024 / 1024 / 1024);

        if(mbr_buf.generic.partition[i].lba_start)
        {
            ret = lseek(fd, mbr_buf.generic.partition[i].lba_start * 512, SEEK_SET);
            if(ret == -1)
            {
                perror("lseek is failed...\n");
                return;
            }

            ret = read(fd, &pbr_buf, sizeof(pbr_t));
            if(ret == -1)
            {
                perror("mmc read failed...\n");
                return;
            }

            vfat_dump("boot_code", (uint8_t*)&pbr_buf.field.boot_code, sizeof(pbr_buf.field.boot_code));
            vfat_dump("oem_id", (uint8_t*)&pbr_buf.field.oem_id, sizeof(pbr_buf.field.oem_id));
            printf(" > bytes_per_sector                 : %x\n", pbr_buf.field.bpp.bytes_per_sector);
            printf(" > sector_per_cluster               : %x\n", pbr_buf.field.bpp.sector_per_cluster);
            printf(" > reserved_sector_count            : %x\n", pbr_buf.field.bpp.reserved_sector_count);
            printf(" > num_of_fat_tables                : %x\n", pbr_buf.field.bpp.num_of_fat_tables);
            printf(" > root_direcotry_entry_count       : %x\n", pbr_buf.field.bpp.root_direcotry_entry_count);
            printf(" > total_sector16                   : %x\n", pbr_buf.field.bpp.total_sector16);
            printf(" > media_type                       : %x\n", pbr_buf.field.bpp.media_type);
            printf(" > fat_size16                       : %x\n", pbr_buf.field.bpp.fat_size16);
            printf(" > num_of_heads                     : %x\n", pbr_buf.field.bpp.num_of_heads);
            printf(" > hidden_sector                    : %x\n", pbr_buf.field.bpp.hidden_sector);
            printf(" > total_sector32                   : %x\n", pbr_buf.field.bpp.total_sector32);
            printf(" > fat_size32                       : %x\n", pbr_buf.field.bpp.fat_size32);
            printf(" > file_system_version              : %x\n", pbr_buf.field.bpp.file_system_version);
            printf(" > root_directory_cluster           : %x\n", pbr_buf.field.bpp.root_directory_cluster);
            printf(" > file_system_information_sector   : %x\n", pbr_buf.field.bpp.file_system_information_sector);
            printf(" > backup_boot_record_sector        : %x\n", pbr_buf.field.bpp.backup_boot_record_sector);
            printf(" > drive_number                     : %x\n", pbr_buf.field.bpp.drive_number);
            printf(" > boot_signature                   : %x\n", pbr_buf.field.bpp.boot_signature);
            vfat_dump("volume_id", (uint8_t*)&pbr_buf.field.bpp.volume_id, sizeof(pbr_buf.field.bpp.volume_id));
            vfat_dump("volume_lable", (uint8_t*)&pbr_buf.field.bpp.volume_lable, sizeof(pbr_buf.field.bpp.volume_lable));
            vfat_dump("file_system_type", (uint8_t*)&pbr_buf.field.bpp.file_system_type, sizeof(pbr_buf.field.bpp.file_system_type));

            ret = lseek(fd, (mbr_buf.generic.partition[i].lba_start + pbr_buf.field.bpp.file_system_information_sector) * 512, SEEK_SET);
            if(ret == -1)
            {
                perror("lseek is failed...\n");
                return;
            }

            ret = read(fd, &fsinfo_buf, sizeof(FSINFO_t));
            if(ret == -1)
            {
                perror("mmc read failed...\n");
                return;
            }

            printf(" > signature1                       : %x\n", fsinfo_buf.field.signature1);
            printf(" > signature2                       : %x\n", fsinfo_buf.field.signature2);
            printf(" > num_of_free_clusters             : %x\n", fsinfo_buf.field.num_of_free_clusters);
            printf(" > next_free_cluster                : %x\n", fsinfo_buf.field.next_free_cluster);
            printf(" > signature3                       : %x\n", fsinfo_buf.field.signature3);
        }
    }
    printf("signature: %x\n", mbr_buf.generic.signature);

}

void vfat_dummy(void)
{
    printf("[%s:%u]\n", __func__, __LINE__);
}
