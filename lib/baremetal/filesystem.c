// TEMPORARY filesystem based on bootloader modules

#ifndef EFFEKT_FS_C
#define EFFEKT_FS_C

static struct limine_file *c_fs_file(Int index)
{
	struct limine_module_response *response = module_request.response;

	if (!response || index < 0 || (uint64_t)index >= response->module_count)
		return 0;

	return response->modules[index];
}

Int c_fs_count(void);
Int c_fs_count(void)
{
	struct limine_module_response *response = module_request.response;

	return !response ? 0 : (Int)response->module_count;
}

Int c_fs_size(Int index);
Int c_fs_size(Int index)
{
	struct limine_file *file = c_fs_file(index);

	return !file ? 0 : (Int)file->size;
}

struct Pos c_fs_path(Int index);
struct Pos c_fs_path(Int index)
{
	struct limine_file *file = c_fs_file(index);

	if (!file)
		return c_bytearray_new(0);

	return c_bytearray_from_nullterminated_string(file->path);
}

struct Pos c_fs_read(Int index);
struct Pos c_fs_read(Int index)
{
	struct limine_file *file = c_fs_file(index);

	if (!file)
		return c_bytearray_new(0);

	return c_bytearray_construct(file->size,
				     (const uint8_t *)file->address);
}

#endif
