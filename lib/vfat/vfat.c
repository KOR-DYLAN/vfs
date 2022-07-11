#include <assert.h>
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

static void vfat_dump(const char* msg, uint32_t offset, uint8_t* buf, size_t len)
{
    size_t i, j;
    uint8_t* pos;

    printf(" > %s\n", msg);
    for(i = 0; i < len; i += 16)
    {
        pos = &buf[i];
        printf("%08x : ", (uint32_t)(offset + i));

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

#if 1
static int mmc_read(int fd, size_t offset, uint8_t *buf, size_t len)
{
    int ret ;

    ret = lseek(fd, offset, SEEK_SET);
    if(ret == -1)
    {
        perror("lseek is failed...\n");
        return ret;
    }

    ret = read(fd, buf, len);
    if(ret == -1)
    {
        perror("mmc read failed...\n");
        return ret;
    }

    return 0;
}

void vfat_parse_mbr(int fd)
{
    int ret;
    int i;
    
    for(i = 0 ; i < 4; i++)
    {
        mbr_t mbr_buf;
        ret = mmc_read(fd, 0, (uint8_t*)&mbr_buf, sizeof(mbr_buf));
        assert(ret == 0);

        if(mbr_buf.generic.partition[i].lba_start != 0)
        {
            printf(">> partition[%d]\n", i);
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

            uint32_t bootcode;
            size_t part_offset = mbr_buf.generic.partition[i].lba_start * DEFAULT_LOGICAL_SECTOR_SIZE;
            ret = mmc_read(fd, part_offset, (uint8_t*)&bootcode, sizeof(bootcode));
            assert(ret == 0);

            bootcode &= VFAT_BOOT_CODE_MASK;
            if(bootcode == VFAT_BOOT_CODE_FAT32)
            {
                vfat_t *vfat_instance;

                vfat_instance = (vfat_t*)malloc(sizeof(vfat_t));
                assert(vfat_instance != NULL);

                vfat_instance->pbr = (pbr_t*)malloc(sizeof(pbr_t));
                assert(vfat_instance->pbr != NULL);

                ret = mmc_read(fd, part_offset, (uint8_t*)vfat_instance->pbr, sizeof(pbr_t));
                assert(ret == 0);

                vfat_dump("boot_code", 0, (uint8_t*)vfat_instance->pbr->field.boot_code, 3);
                vfat_dump("oem_id", 0, (uint8_t*)vfat_instance->pbr->field.oem_id, 8);
                printf(" > bytes_per_sector                 : %x\n", vfat_instance->pbr->field.bpp.bytes_per_sector);
                printf(" > sector_per_cluster               : %x\n", vfat_instance->pbr->field.bpp.sector_per_cluster);
                printf(" > reserved_sector_count            : %x\n", vfat_instance->pbr->field.bpp.reserved_sector_count);
                printf(" > num_of_fat_tables                : %x\n", vfat_instance->pbr->field.bpp.num_of_fat_tables);
                printf(" > root_direcotry_entry_count       : %x\n", vfat_instance->pbr->field.bpp.root_direcotry_entry_count);
                printf(" > total_sector16                   : %x\n", vfat_instance->pbr->field.bpp.total_sector16);
                printf(" > media_type                       : %x\n", vfat_instance->pbr->field.bpp.media_type);
                printf(" > fat_size16                       : %x\n", vfat_instance->pbr->field.bpp.fat_size16);
                printf(" > num_of_heads                     : %x\n", vfat_instance->pbr->field.bpp.num_of_heads);
                printf(" > hidden_sector                    : %x\n", vfat_instance->pbr->field.bpp.hidden_sector);
                printf(" > total_sector32                   : %x\n", vfat_instance->pbr->field.bpp.total_sector32);
                printf(" > fat_size32                       : %x\n", vfat_instance->pbr->field.bpp.fat_size32);
                printf(" > file_system_version              : %x\n", vfat_instance->pbr->field.bpp.file_system_version);
                printf(" > root_directory_cluster           : %x\n", vfat_instance->pbr->field.bpp.root_directory_cluster);
                printf(" > file_system_information_sector   : %x\n", vfat_instance->pbr->field.bpp.file_system_information_sector);
                printf(" > backup_boot_record_sector        : %x\n", vfat_instance->pbr->field.bpp.backup_boot_record_sector);
                printf(" > drive_number                     : %x\n", vfat_instance->pbr->field.bpp.drive_number);
                printf(" > boot_signature                   : %x\n", vfat_instance->pbr->field.bpp.boot_signature);
                vfat_dump("volume_id", 0, (uint8_t*)vfat_instance->pbr->field.bpp.volume_id, 4);
                vfat_dump("volume_lable", 0, (uint8_t*)vfat_instance->pbr->field.bpp.volume_lable, 11);
                vfat_dump("file_system_type", 0, (uint8_t*)vfat_instance->pbr->field.bpp.file_system_type, 8);

                size_t fsinfo_offset =
                    part_offset + 
                    (vfat_instance->pbr->field.bpp.file_system_information_sector * vfat_instance->pbr->field.bpp.bytes_per_sector);

                vfat_instance->fs_info = (FSINFO_t*)malloc(sizeof(FSINFO_t));
                assert(vfat_instance->fs_info != NULL);

                ret = mmc_read(fd, fsinfo_offset, (uint8_t*)vfat_instance->fs_info, sizeof(FSINFO_t));
                assert(ret == 0);

                printf(" > signature1                       : %x\n", vfat_instance->fs_info->field.signature1);
                printf(" > signature2                       : %x\n", vfat_instance->fs_info->field.signature2);
                printf(" > num_of_free_clusters             : %x\n", vfat_instance->fs_info->field.num_of_free_clusters);
                printf(" > next_free_cluster                : %x\n", vfat_instance->fs_info->field.next_free_cluster);
                printf(" > signature3                       : %x\n", vfat_instance->fs_info->field.signature3);

                vfat_instance->size_of_fat =
                    vfat_instance->pbr->field.bpp.fat_size32 * vfat_instance->pbr->field.bpp.bytes_per_sector;
                vfat_instance->cluster_per_bytes =
                    vfat_instance->pbr->field.bpp.bytes_per_sector * vfat_instance->pbr->field.bpp.sector_per_cluster;

                vfat_instance->fat1_base_offset = 
                    part_offset + 
                    (vfat_instance->pbr->field.bpp.reserved_sector_count * vfat_instance->pbr->field.bpp.bytes_per_sector);
                vfat_instance->fat2_base_offset = 
                    vfat_instance->fat1_base_offset + 
                    vfat_instance->size_of_fat;
                vfat_instance->cluster_base_offset = 
                    vfat_instance->fat2_base_offset +
                    vfat_instance->size_of_fat;

                printf(" > fat length                       : %x\n", vfat_instance->size_of_fat);
                printf(" > cluster length                   : %x\n", vfat_instance->cluster_per_bytes);
                printf(" > fat1 offset                      : %x\n", vfat_instance->fat1_base_offset);
                printf(" > fat2 offset                      : %x\n", vfat_instance->fat2_base_offset);
                printf(" > root offset                      : %x\n", vfat_instance->cluster_base_offset);

                uint8_t buf[4096];

                ret = mmc_read(fd, vfat_instance->fat1_base_offset, (uint8_t*)buf, sizeof(buf));
                assert(ret == 0);
                vfat_dump("fat1", vfat_instance->fat1_base_offset, buf, 16);

                ret = mmc_read(fd, vfat_instance->fat2_base_offset, (uint8_t*)buf, sizeof(buf));
                assert(ret == 0);
                vfat_dump("fat2", vfat_instance->fat2_base_offset, buf, 16);

                ret = mmc_read(fd, vfat_instance->cluster_base_offset, (uint8_t*)buf, sizeof(buf));
                assert(ret == 0);
                vfat_dump("cluster", vfat_instance->cluster_base_offset, buf, 256);

                ROOT_ENRTRY_t *root_entry = (ROOT_ENRTRY_t *)&buf[64];
                vfat_dump("file_name", 0, root_entry->field.file_name, sizeof(root_entry->field.file_name));
                vfat_dump("extension", 0, root_entry->field.extension, sizeof(root_entry->field.extension));
                vfat_dump("attribute", 0, &root_entry->field.attribute, sizeof(root_entry->field.attribute));
                vfat_dump("windows_nt", 0, &root_entry->field.windows_nt, sizeof(root_entry->field.windows_nt));
                vfat_dump("creation_time", 0, root_entry->field.creation_time, sizeof(root_entry->field.creation_time));
                vfat_dump("creation_date", 0, root_entry->field.creation_date, sizeof(root_entry->field.creation_date));
                vfat_dump("last_access_date", 0, root_entry->field.last_access_date, sizeof(root_entry->field.last_access_date));
                vfat_dump("first_cluster_upper", 0, &root_entry->field.first_cluster_upper, sizeof(root_entry->field.first_cluster_upper));
                vfat_dump("last_modified_time", 0, root_entry->field.last_modified_time, sizeof(root_entry->field.last_modified_time));
                vfat_dump("last_modified_date", 0, root_entry->field.last_modified_date, sizeof(root_entry->field.last_modified_date));
                vfat_dump("first_cluster_lower", 0, &root_entry->field.first_cluster_lower, sizeof(root_entry->field.first_cluster_lower));
                vfat_dump("file_size", 0, &root_entry->field.file_size, sizeof(root_entry->field.file_size));
            }
            else
            {
                printf("bootcode %x is not supported...", bootcode);
                continue;;
            }
        }
    }
}
#else
void vfat_parse_mbr(int fd)
{
    size_t i;
    off_t ret;
    mbr_t mbr_buf;

    size_t pbr_offset;
    pbr_t pbr_buf;

    size_t fsinfo_offset;
    FSINFO_t fsinfo_buf;

    size_t fat1_offset, fat2_offset;

    size_t root_entry_offset;
    ROOT_ENRTRY_t root_entry_buf;

    uint8_t dump_buf[4096];

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
            pbr_offset = mbr_buf.generic.partition[i].lba_start * 512;
            ret = lseek(fd, pbr_offset, SEEK_SET);
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

            fsinfo_offset = 
                pbr_offset + 
                (pbr_buf.field.bpp.file_system_information_sector * pbr_buf.field.bpp.bytes_per_sector);
            ret = lseek(fd, fsinfo_offset, SEEK_SET);
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

            fat1_offset = 
                pbr_offset + 
                (pbr_buf.field.bpp.reserved_sector_count * pbr_buf.field.bpp.bytes_per_sector);
            ret = lseek(fd, fat1_offset, SEEK_SET);
            if(ret == -1)
            {
                perror("lseek is failed...\n");
                return;
            }

            ret = read(fd, dump_buf, sizeof(dump_buf));
            if(ret == -1)
            {
                perror("mmc read failed...\n");
                return;
            }
            vfat_dump("fat1_offset", dump_buf, sizeof(dump_buf));

            
            fat2_offset =
                fat1_offset +
                (pbr_buf.field.bpp.fat_size32 * pbr_buf.field.bpp.bytes_per_sector);
            ret = lseek(fd, fat2_offset, SEEK_SET);
            if(ret == -1)
            {
                perror("lseek is failed...\n");
                return;
            }

            ret = read(fd, dump_buf, sizeof(dump_buf));
            if(ret == -1)
            {
                perror("mmc read failed...\n");
                return;
            }
            vfat_dump("fat2_offset", dump_buf, sizeof(dump_buf));

            root_entry_offset = 
                fat2_offset +
                (pbr_buf.field.bpp.fat_size32 * pbr_buf.field.bpp.bytes_per_sector);
                // (pbr_buf.field.bpp.root_directory_cluster * pbr_buf.field.bpp.sector_per_cluster * pbr_buf.field.bpp.bytes_per_sector);
            ret = lseek(fd, root_entry_offset, SEEK_SET);
            if(ret == -1)
            {
                perror("lseek is failed...\n");
                return;
            }

            ret = read(fd, &dump_buf, sizeof(dump_buf));
            if(ret == -1)
            {
                perror("mmc read failed...\n");
                return;
            }
            vfat_dump("root_entry_offset", dump_buf, sizeof(dump_buf));
        }
    }
    printf("signature: %x\n", mbr_buf.generic.signature);

}
#endif

uint32_t vfat_get_cluster_offset(vfat_t *instance, uint32_t cluster_num)
{
    assert(instance != NULL);

    return instance->cluster_base_offset +
        (instance->cluster_per_bytes * cluster_num);
}
