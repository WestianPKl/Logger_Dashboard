import { memo } from 'react'

export default memo(({ data }: { data: any }) => {
	return (
		<div>
			{data.background.split('/')[3] != 'null' ? (
				<img src={data.background} alt='Floor layout picture' />
			) : (
				<p>Add layout picture</p>
			)}
		</div>
	)
})
