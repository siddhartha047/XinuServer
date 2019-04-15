#include<xinu.h>

void initialize_backing_store(void){
	intmask mask=disable();

	for(int i=0;i<MAX_BS_ENTRIES;i++){
		backing_store_map_tab[i].pid=-1;
		backing_store_map_tab[i].vpn=0;
		backing_store_map_tab[i].npages=0;
		backing_store_map_tab[i].bsid=-1;
		backing_store_map_tab[i].allocated=0;
	}
	restore(mask);
	return;
}

int32 add_bs_map(pid32 pid, uint32 vpn, uint32 npages, bsd_t bsid){
	intmask mask=disable();

	backing_store_map *bs_map_entry;

	if(bstab[bsid].isallocated==FALSE||backing_store_map_tab[bsid].allocated==1){
		restore(mask);
		return SYSERR;
	}

	bs_map_entry=&backing_store_map_tab[bsid];
	bs_map_entry->pid=pid;
	bs_map_entry->vpn=vpn;
	bs_map_entry->npages=npages;
	bs_map_entry->bsid=bsid;
	bs_map_entry->allocated=1;

	restore(mask);
	return OK;
}

int32 remove_bs_map(pid32 pid){
	intmask mask=disable();

	backing_store_map *bs_map_entry;

	for(int i=0;i<MAX_BS_ENTRIES;i++){
		bs_map_entry=&backing_store_map_tab[i];

		if(bs_map_entry->allocated==1 && bs_map_entry->pid==pid){
			int32 status= deallocate_bs(bs_map_entry->bsid);
			if(status==SYSERR){
				restore(mask);
				return SYSERR;
			}

			bs_map_entry->pid=-1;
			bs_map_entry->vpn=0;
			bs_map_entry->npages=0;
			bs_map_entry->bsid=-1;
			bs_map_entry->allocated=0;

		}
	}

	restore(mask);

	return OK;
}


backing_store_map* get_bs_map(pid32 pid, uint32 vpn){
	intmask mask=disable();
	backing_store_map* bs_map_entry;
	struct bs_entry *bs_map;

	for(int i=0;i<MAX_BS_ENTRIES;i++){
		bs_map=&bstab[i];
		bs_map_entry=&backing_store_map_tab[i];

		if( !((bs_map->isallocated==0) || 
			(bs_map_entry->allocated==0)||
			(bs_map_entry->pid!=pid)||
			(vpn<bs_map_entry->vpn)||
			(vpn>=(bs_map_entry->vpn+bs_map_entry->npages)))){
			restore(mask);
			return bs_map_entry;
		}		
	}	

	restore(mask);
	return (backing_store_map *)NULL;
}