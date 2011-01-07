/* Print some basic Wii/GC ISO information.
 *
 * @author: gammy
 *
 * Resources used:
 *	http://wiibrew.org/wiki/Wii_Disc (offsets & magic)
 * 	wiidisc.c from wbfs (big endian wrapper, logic)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define OFFSET_PART2	0x018
#define OFFSET_REGION	0x4E000
#define OFFSET_MAGIC	0x4FFFC

#define DISC_MAGIC	0xC3F81A8E
#define WII_MAGIC	0x5D1C9EA3
#define GCC_MAGIC	0xC2339F3D

uint32_t _be32(uint8_t *p) {
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

char *get_region_str(uint8_t region) {
	switch(region) {
		case 'E': return "USA";
		case 'P': return "PAL";
		case 'J': return "JAP";
		case 'K': return "KOR";
	}

	return("Unknown");
}

int main(int argc, const char *argv[]) {

	assert(argc == 2);

	char *file = (char *)argv[1];

	FILE *fp = fopen(file, "rb");

	if(! fp) {
		perror("fopen");
		return(EXIT_FAILURE);
	}

	rewind(fp);
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);

	if(size == -1) {
		perror("ftell");
		return(EXIT_FAILURE);
	}
	
	printf("%ld bytes (%ld MiB)\n\n", size, (size / 1024) / 1024);

	struct { 
		uint8_t id;
		uint8_t game_code[2];
		uint8_t region_code;
		uint8_t maker_code[2];
		uint8_t disc_number; // unused in our code
		uint8_t version;
	} part1;

	struct {
		uint8_t wii_magic[4];
		uint8_t gc_magic[4];
		uint8_t title[64];
	} part2;
	
	uint32_t magic;
	fseek(fp, OFFSET_MAGIC, SEEK_SET);
	fread(&magic, sizeof(uint32_t), 1, fp);

	if(_be32((uint8_t *) &magic) != DISC_MAGIC) {
		fprintf(stderr, "Invalid disc magic(%x != %x) - invalid disc?\n",
			_be32((uint8_t *) &magic), DISC_MAGIC);
		return(EXIT_FAILURE);
	}

	rewind(fp);
	fread(&part1, sizeof(uint8_t), sizeof(part1), fp);

	fseek(fp, OFFSET_PART2, SEEK_SET);
	fread(&part2, sizeof(uint8_t), sizeof(part2), fp);
	
	fclose(fp);

	char game_id[7];
	memcpy(&game_id, &part1, sizeof(game_id) - sizeof(char));
	game_id[6] = 0;

	printf("Title       : %s\n"
	       "Version     : %x ?\n"
	       "Wii/GC game : %s\n\n"
	       "Console code: %c\n"
	       "Game code   : %c%c\n"
	       "Region code : %s (%c)\n"
	       "Maker code  : %c%c\n"
	       "Game ID     = %s\n\n",
	       part2.title,
	       part1.version,
	       _be32((uint8_t *) &part2.wii_magic) == WII_MAGIC ? "Wii" : "GC",
	       part1.id,
	       part1.game_code[0], part1.game_code[1], // ([0] << 8) | [1]
	       get_region_str(part1.region_code), part1.region_code,
	       part1.maker_code[0], part1.maker_code[1], // ([0] << 8) | [1]
	       game_id);

	return(EXIT_SUCCESS);
}
