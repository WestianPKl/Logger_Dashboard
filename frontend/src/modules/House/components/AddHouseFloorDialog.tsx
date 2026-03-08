import { useEffect, useState, useRef } from 'react'
import {
	Box,
	TextField,
	Button,
	Dialog,
	DialogActions,
	DialogContent,
	DialogTitle,
	useMediaQuery,
	useTheme,
} from '@mui/material'
import type { IAddHouseFoorProps } from '../scripts/IHouse'
import HouseSelect from '../../../components/HouseSelect'
import type { HouseClass } from '../scripts/HouseClass'
import classes from './AddHouseFloorDialog.module.css'

export default function AddHouseFloorDialog({
	edit,
	isDashboard,
	dashboardData,
	selectedItems,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddHouseFoorProps) {
	const [name, setName] = useState('')
	const [house, setHouse] = useState<HouseClass | null>(null)
	const [enteredImg, setEnteredImg] = useState<File | string | undefined>(undefined)
	const [previewImg, setPreviewImg] = useState<string | undefined>(undefined)
	const [multiple, setMultiple] = useState(false)
	const [itemId, setItemId] = useState<number | undefined>(undefined)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))
	const imgPickerRef = useRef<HTMLInputElement | null>(null)

	useEffect(() => {
		if (isDashboard && dashboardData) {
			setHouse(dashboardData)
		} else {
			setHouse(null)
		}
		if (edit) {
			if (selectedItems?.length === 1) {
				const item = selectedItems[0]
				setName(item.name || '')
				setHouse(item.house || null)
				setItemId(item.id)
				setEnteredImg(item.layout)
				setMultiple(false)
			} else {
				setName('')
				setHouse(null)
				setItemId(undefined)
				setEnteredImg(undefined)
				setMultiple(true)
			}
		} else {
			setName('')
			setHouse(isDashboard && dashboardData ? dashboardData : null)
			setItemId(undefined)
			setEnteredImg(undefined)
			setMultiple(false)
		}
		setPreviewImg(undefined)
	}, [openAddDialog, edit, selectedItems, isDashboard, dashboardData])

	useEffect(() => {
		if (!enteredImg) {
			setPreviewImg(undefined)
			return
		}
		if (enteredImg instanceof File) {
			const fileReader = new FileReader()
			fileReader.onload = () => setPreviewImg(fileReader.result as string)
			fileReader.readAsDataURL(enteredImg)
		} else if (typeof enteredImg === 'string' && enteredImg.length > 0) {
			setPreviewImg(`${import.meta.env.VITE_API_IP}/${enteredImg}?w=100&h=100&format=webp`)
		}
	}, [enteredImg])

	function pickImg(): void {
		imgPickerRef.current?.click()
	}

	function imgHandler(e: React.ChangeEvent<HTMLInputElement>): void {
		const file = e.target.files?.[0]
		if (file) setEnteredImg(file)
	}

	function closeDialog(): void {
		handleCloseAdd()
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!edit) {
			addItemHandler({
				name,
				layout: enteredImg,
				houseId: house?.id,
			})
		} else if (edit && multiple) {
			addItemHandler(
				selectedItems?.map(item => ({
					id: item.id,
					name: item.name,
					layout: item.layout,
					houseId: item.houseId,
				})) || [],
			)
		} else if (edit && !multiple) {
			addItemHandler([
				{
					id: itemId,
					name,
					layout: enteredImg,
					houseId: house?.id,
				},
			])
		}
		closeDialog()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>{edit ? 'Edit house floor' : 'Add house floor'}</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
						<TextField
							sx={{ mt: 1, width: isMobile ? 200 : 400 }}
							id='name'
							label='Name'
							onChange={e => setName(e.target.value)}
							disabled={multiple}
							value={name}
							autoFocus
						/>
						<HouseSelect getItem={setHouse} item={house} disabled={isDashboard || multiple} />
						<input
							type='file'
							id='img'
							ref={imgPickerRef}
							style={{ display: 'none' }}
							accept='.jpg,.png,.jpeg'
							onChange={imgHandler}
						/>
						<div className={classes.imgHolder}>
							{previewImg && (
								<div className={classes.img}>
									<img src={previewImg} alt='Preview' className={classes.img} />
								</div>
							)}
							<div className={classes.img_action}>
								<Button type='button' size={isMobile ? 'small' : 'medium'} onClick={pickImg}>
									Choose layout!
								</Button>
							</div>
						</div>
					</Box>
				</DialogContent>
				<DialogActions>
					<Button variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={closeDialog}>
						Cancel
					</Button>
					<Button
						variant='outlined'
						size={isMobile ? 'small' : 'medium'}
						type='submit'
						disabled={!name.trim() || !house || (edit && multiple)}>
						{edit ? 'Save' : 'Add'}
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
